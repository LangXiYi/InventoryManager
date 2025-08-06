// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "InvSys_InventoryItemDefinition.h"
#include "InvSys_InventoryItemInstance.h"
#include "NativeGameplayTags.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InvSys_ContainerList.generated.h"

class UInvSys_InventoryComponent;
class UInvSys_InventoryItemInstance;
struct FInvSys_ContainerEntry;

USTRUCT(BlueprintType)
struct FInvSys_InventoryItemChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TObjectPtr<UInvSys_InventoryComponent> InvComp;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FGameplayTag InventoryObjectTag;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	UInvSys_InventoryItemInstance* ItemInstance = nullptr;
};

USTRUCT(BlueprintType)
struct FInvSys_InventoryStackChangeMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TObjectPtr<UInvSys_InventoryComponent> InvComp;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FGameplayTag InventoryObjectTag;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	UInvSys_InventoryItemInstance* ItemInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	int32 StackCount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	int32 Delta = 0;
};

/*
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryStackChange, FInvSys_InventoryStackChangeMessage);*/
DECLARE_DELEGATE_TwoParams(FOnInventoryItemChange, UInvSys_InventoryItemInstance*, bool);
// 由容器广播给外部。
DECLARE_DELEGATE_TwoParams(FOnContainerEntryChange, const FInvSys_ContainerEntry&, bool);

/** Example */
USTRUCT()
struct FInvSys_ContainerEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	friend struct FInvSys_ContainerList;

public:
	UPROPERTY()
	TObjectPtr<UInvSys_InventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;
	
	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;

public:
	/**
	 * Called right before deleting element during replication.
	 * 
	 * @param InArraySerializer	Array serializer that owns the item and has triggered the replication call
	 * 
	 * NOTE: intentionally not virtual; invoked via templated code, @see FExampleItemEntry
	 */
	FORCEINLINE void PreReplicatedRemove(const struct FFastArraySerializer& InArraySerializer)
	{
		Instance->ReplicateState = EInvSys_ReplicateState::PreRemove;
		Instance->PreReplicatedRemove();
	}
	/**
	 * Called after adding and serializing a new element
	 *
	 * @param InArraySerializer	Array serializer that owns the item and has triggered the replication call
	 * 
	 * NOTE: intentionally not virtual; invoked via templated code, @see FExampleItemEntry
	 */
	FORCEINLINE void PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer)
	{
		Instance->ReplicateState = EInvSys_ReplicateState::PostAdd;
	}
	/**
	 * Called after updating an existing element with new data
	 *
	 * @param InArraySerializer	Array serializer that owns the item and has triggered the replication call
	 * NOTE: intentionally not virtual; invoked via templated code, @see FExampleItemEntry
	 */
	FORCEINLINE void PostReplicatedChange(const struct FFastArraySerializer& InArraySerializer)
	{
		Instance->ReplicateState = EInvSys_ReplicateState::PostChange;
	}

	FString GetDebugString() const;

	template<class T = UInvSys_InventoryItemInstance>
	T* GetInstance() const
	{
		return (T*)Instance;
	}

	bool IsValid() const
	{
		return Instance != nullptr;
	}
};

/*
 * 库存列表
 * 问题：在修改内部成员对象的属性后，在添加一个新的成员对象，会出现问题，新成员会优先发送给客户端，后面才是修改后的属性！！！
 */
USTRUCT()
struct BASEINVENTORYSYSTEM_API FInvSys_ContainerList : public FFastArraySerializer
{
	GENERATED_BODY()

	friend class UInvSys_BaseEquipContainerObject;

public:
	FInvSys_ContainerList() { }

	FInvSys_ContainerList(UInvSys_BaseInventoryFragment* InOwnerObject) : InventoryFragment(InOwnerObject)
	{
		SetDeltaSerializationEnabled(false);
	}

public:
	UPROPERTY()
	TArray<FInvSys_ContainerEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UInvSys_BaseInventoryFragment> InventoryFragment;

public:
	template<class T, class... Arg>
	T* AddDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount, const Arg&... Args)
	{
		check(ItemDef)
		check(InventoryFragment)
		check(InventoryFragment->HasAuthority());
		FInvSys_ContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
		T* Result = NewObject<T>(InventoryFragment->GetInventoryComponent());
		// Result->SetInventoryComponent(InventoryFragment->GetInventoryComponent());
		Result->Entry_Private = &NewEntry;
		// Result->Container_Private = this;
		Result->SetItemDefinition(ItemDef);
		Result->SetItemUniqueID(FGuid::NewGuid());
		// 这两个属性表明了这个对象的最基础的位置信息
		Result->SetSlotTag(InventoryFragment->GetInventoryObjectTag());

		for (const UInvSys_InventoryItemFragment* Fragment : GetDefault<UInvSys_InventoryItemDefinition>(ItemDef)->GetFragments())
		{
			if (Fragment != nullptr)
			{
				Fragment->OnInstanceCreated(NewEntry.Instance);
			}
		}
		
		NewEntry.Instance = Result;
		NewEntry.StackCount = StackCount;
		//执行可变参数模板，将参数列表中的值赋予目标对象。
		int32 Arr[] = {0, (InitItemInstanceProps<T>(Result, Args), 0)...}; 
		
		MarkItemDirty(NewEntry);
		
		if (InventoryFragment && InventoryFragment->GetNetMode() != NM_DedicatedServer)
		{
			Result->BroadcastAddItemInstanceMessage();
			// BroadcastAddEntryMessage(NewEntry);
		}
		check(Result)
		return Result;
	}

	/**
	 * 添加其他物品实例
	 */
	template<class T, class... Arg>
	bool AddInstance(UInvSys_InventoryItemInstance* Instance, const Arg&... Args)
	{
		check(Instance)
		// 更新物品的基础信息
		Instance->SetSlotTag(InventoryFragment->GetInventoryObjectTag());
		// Instance->SetInventoryComponent(InventoryFragment->GetInventoryComponent());
		//执行可变参数模板，将参数列表中的值赋予目标对象。
		int32 Arr[] = {0, (InitItemInstanceProps<T>(Instance, Args), 0)...};

		FInvSys_ContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
		Instance->Entry_Private = &NewEntry;
		// Instance->Container_Private = InventoryFragment;

		NewEntry.Instance = Instance;
		MarkItemDirty(NewEntry);
		check(InventoryFragment)
		if (InventoryFragment && InventoryFragment->GetNetMode() != NM_DedicatedServer)
		{
			Instance->BroadcastAddItemInstanceMessage();
			// BroadcastAddEntryMessage(NewEntry);
		}
		return true;
	}

	void RemoveAll();
	
	bool RemoveEntry(UInvSys_InventoryItemInstance* Instance);

	void RemoveAt(int32 Index);

	// bool UpdateEntryStackCount(UInvSys_InventoryItemInstance* Instance, int32 NewCount);

	template<class T = UInvSys_InventoryItemInstance>
	TArray<T*> GetAllItems() const
	{
		TArray<T*> Results;
		Results.Reserve(Entries.Num());
		for (const FInvSys_ContainerEntry& Entry : Entries)
		{
			if (Entry.Instance != nullptr && Entry.Instance->IsA<T>()) //@TODO: Would prefer to not deal with this here and hide it further?
			{
				Results.Add((T*)Entry.Instance);
			}
		}
		return Results;
	}

	UInvSys_InventoryItemInstance* FindItem(FGuid ItemUniqueID) const;

public:
	/**
	 * PreRemove 发生在对象的属性复制之前
	 */
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	/**
	 * PostAdd 发生在对象的属性复制之前
	 */
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	/**
	 * PostChange 发生在对象的属性复制之前
	 */
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	// void PostReplicatedReceive(const FPostReplicatedReceiveParameters& Parameters);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInvSys_ContainerEntry, FInvSys_ContainerList>(Entries, DeltaParms, *this);
	}

	FORCEINLINE FOnContainerEntryChange& OnContainerEntryAddedDelegate()
	{
		return OnContainerEntryAdded;
	}

	FORCEINLINE FOnContainerEntryChange& OnContainerEntryRemoveDelegate()
	{
		return OnContainerEntryRemove;
	}

	bool Contains(UInvSys_InventoryItemInstance* ItemInstance) const
	{
		for (FInvSys_ContainerEntry Entry : Entries)
		{
			if (Entry.Instance == ItemInstance)
			{
				return true;
			}
		}
		return false;
	}

	/*FORCEINLINE FOnInventoryStackChange& OnInventoryStackChangeDelegate()
	{
		return OnInventoryStackChange;
	}*/

protected:

private:
	template<class C, class V>
	void InitItemInstanceProps(UInvSys_InventoryItemInstance* ItemInstance, const V& Value)
	{
		if (ItemInstance != nullptr && ItemInstance->IsA<C>())
		{
			((C*)ItemInstance)->InitItemInstanceProps(Value);	// ItemInstance 需要创建对应的函数处理该值
		}
	}

protected:
	FOnContainerEntryChange OnContainerEntryAdded;

	FOnContainerEntryChange OnContainerEntryRemove;

	/*FOnInventoryStackChange OnInventoryStackChange;*/
};

template<>
struct TStructOpsTypeTraits< FInvSys_ContainerList > : public TStructOpsTypeTraitsBase2< FInvSys_ContainerList >
{
	enum { WithNetDeltaSerializer = true };
};