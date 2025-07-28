// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	FString GetDebugString() const;

	template<class T = UInvSys_InventoryItemInstance>
	T* GetInstance() const
	{
		return (T*)Instance;
	}
};

/*
 * 库存列表
 */
USTRUCT()
struct BASEINVENTORYSYSTEM_API FInvSys_ContainerList : public FFastArraySerializer
{
	GENERATED_BODY()

	friend class UInvSys_BaseEquipContainerObject;

public:
	FInvSys_ContainerList()
	{	}

	FInvSys_ContainerList(UInvSys_BaseInventoryFragment* InOwnerObject) : OwnerObject(InOwnerObject)
	{	}

	void BroadcastAddEntryMessage(const FInvSys_ContainerEntry& Entry);
	
	void BroadcastRemoveEntryMessage(const FInvSys_ContainerEntry& Entry);

	void BroadcastStackChangeMessage(const FInvSys_ContainerEntry& Entry, int32 OldCount, int32 NewCount); // 广播堆叠数量变化事件
	
public:
	UPROPERTY()
	TArray<FInvSys_ContainerEntry> Entries;

	// UPROPERTY()
	// TArray<FInvSys_ContainerEntry> PostRepEntries; // 在RepSubObject中使用

	UPROPERTY(NotReplicated)
	TObjectPtr<UInvSys_BaseInventoryFragment> OwnerObject;
public:
	template<class T, class... Arg>
	T* AddEntry(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount, const Arg&... Args)
	{
		if (ItemDef == nullptr || OwnerObject == nullptr)
		{
			checkNoEntry();
			return nullptr;
		}
		
		check(OwnerObject->HasAuthority());
		FInvSys_ContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
		T* Result = NewObject<T>(OwnerObject->GetInventoryComponent());
		Result->SetItemDefinition(ItemDef);
		Result->SetItemUniqueID(FGuid::NewGuid());
		// 这两个属性表明了这个对象的最基础的位置信息
		Result->SetSlotTag(OwnerObject->GetInventoryObjectTag());
		Result->SetInventoryComponent(OwnerObject->GetInventoryComponent());

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
		int32 Arr[] = {0, (InitItemInstanceProps(Result, Args), 0)...}; 
		
		MarkItemDirty(NewEntry);
		
		if (OwnerObject && OwnerObject->GetNetMode() != NM_DedicatedServer)
		{
			BroadcastAddEntryMessage(NewEntry);
		}
		return Result;
	}

	/**
	 * 添加其他物品实例
	 */
	template<class T, class... Arg>
	bool AddEntry(T* Instance, const Arg&... Args)
	{
		if (Instance)
		{
			// 更新物品的基础信息
			Instance->SetSlotTag(OwnerObject->GetInventoryObjectTag());
			Instance->SetInventoryComponent(OwnerObject->GetInventoryComponent());
			//执行可变参数模板，将参数列表中的值赋予目标对象。

			int32 Arr[] = {0, (InitItemInstanceProps(Instance, Args), 0)...};


			FInvSys_ContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
			NewEntry.Instance = Instance;
			MarkItemDirty(NewEntry);

			check(OwnerObject)
			if (OwnerObject && OwnerObject->GetNetMode() != NM_DedicatedServer)
			{
				BroadcastAddEntryMessage(NewEntry);
			}
			return true;
		}
		return false;
	}

	void RemoveAll();
	
	bool RemoveEntry(UInvSys_InventoryItemInstance* Instance);

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
	//~FFastArraySerializer contract // 仅非Server的客户端接收该数据
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract
	
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
	void InitItemInstanceProps(C* ItemInstance, const V& Value)
	{
		if (ItemInstance != nullptr)
		{
			ItemInstance->InitItemInstanceProps(Value);	// ItemInstance 需要创建对应的函数处理该值
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