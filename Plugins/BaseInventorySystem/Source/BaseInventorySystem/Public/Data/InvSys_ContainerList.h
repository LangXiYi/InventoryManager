// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "InvSys_InventoryItemInstance.h"
#include "InvSys_InventoryItemDefinition.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Components/InventoryObject/Fragment/InvSys_BaseInventoryFragment.h"
#include "InvSys_ContainerList.generated.h"

class UInvSys_InventoryComponent;
class UInvSys_InventoryItemInstance;
class UInvSys_BaseInventoryFragment;
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
	 * NOTE: 函数执行优先级在对象的 RepNotify 之前
	 */
	FORCEINLINE void PreReplicatedRemove(const struct FFastArraySerializer& InArraySerializer)
	{
		Instance->ReplicateState = EInvSys_ReplicateState::PreRemove;
		// UE_LOG(LogInventorySystem, Log, TEXT("PreReplicatedRemove: %s "), *Instance->GetName())
		Instance->PreReplicatedRemove();
	}
	/**
	 * Called after adding and serializing a new element
	 *
	 * @param InArraySerializer	Array serializer that owns the item and has triggered the replication call
	 * 
	 * NOTE: intentionally not virtual; invoked via templated code, @see FExampleItemEntry
	 * NOTE: 函数执行优先级在对象的 RepNotify 之前
	 */
	FORCEINLINE void PostReplicatedAdd(const struct FFastArraySerializer& InArraySerializer)
	{
		Instance->ReplicateState = EInvSys_ReplicateState::PostAdd;
		// UE_LOG(LogInventorySystem, Log, TEXT("PostReplicatedAdd: %s "), *Instance->GetName())
	}
	/**
	 * Called after updating an existing element with new data
	 *
	 * @param InArraySerializer	Array serializer that owns the item and has triggered the replication call
	 * NOTE: intentionally not virtual; invoked via templated code, @see FExampleItemEntry
	 * NOTE: 函数执行优先级在对象的 RepNotify 之前
	 */
	FORCEINLINE void PostReplicatedChange(const struct FFastArraySerializer& InArraySerializer)
	{
		Instance->ReplicateState = EInvSys_ReplicateState::PostChange;
		// UE_LOG(LogInventorySystem, Log, TEXT("PostReplicatedChange: %s "), *Instance->GetName())
	}

	FString GetDebugString() const;

	template<class T = UInvSys_InventoryItemInstance>
	T* GetInstance() const
	{
		return (T*)Instance;
	}

	bool IsValid() const
	{
		return Instance->IsValidLowLevel();
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

	typedef int32 SizeType;

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
	/**
	 * 创建物品实例并初始化该物品的属性，添加过程中会广播添加事件。
	 * 注意：可变参数列表要求目标类型必须实现 InitItemInstanceProps 函数，且参数类型一致。
	 */
	template<class T, class... Arg>
	T* AddDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount, const Arg&... Args);

	/**
	 * 将其他物品实例添加到当前容器内，添加过程中会广播添加事件。
	 * 注意：可变参数列表要求目标类型必须实现 InitItemInstanceProps 函数，且参数类型一致。
	 */
	template<class T, class... Arg>
	T* AddInstance(UInvSys_InventoryItemInstance* ItemInstance, const Arg&... Args);

	/** 移除所有物品，移除过程中会广播删除事件。 */
	void RemoveAll();

	/** 将物品实例从当前容器内删除，移除过程中会广播删除事件。 */
	bool RemoveEntry(UInvSys_InventoryItemInstance* Instance);

	/** 获取当前容器的所有物品实例，然后将其转换为指定类型。 */
	template<class T = UInvSys_InventoryItemInstance>
	void GetAllItems(TArray<T*>& OutArray) const
	{
		OutArray.Reserve(Entries.Num());
		for (const FInvSys_ContainerEntry& Entry : Entries)
		{
			if (Entry.Instance && Entry.Instance->IsA<T>())
			{
				OutArray.Add((T*)Entry.Instance);
			}
		}
	}

	/** 根据物品的唯一ID查找物品 */
	UInvSys_InventoryItemInstance* FindItemInstance(FGuid ItemUniqueID) const;

	/** 获取物品实例在当前容器内的索引 */
	int32 FindEntryIndex(UInvSys_InventoryItemInstance* ItemInstance);

	/** 检查目标物品在当前容器内是否存在 */
	bool Contains(UInvSys_InventoryItemInstance* ItemInstance) const;

	/** 检查索引在容器内是否有效 */
	FORCEINLINE bool IsValidIndex(SizeType Index) const
	{
		return Index >= 0 && Index < Entries.Num();
	}

	/** 重载[]运算符，返回指定索引下的容器条目 */
	FORCEINLINE FInvSys_ContainerEntry& operator[](SizeType Index)
	{
		return Entries[Index];
	}

	/** 返回当前容器内存储的物品数量 */
	FORCEINLINE SizeType Num() const
	{
		return Entries.Num();
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInvSys_ContainerEntry, FInvSys_ContainerList>(Entries, DeltaParms, *this);
	}

protected:
	void BroadcastRemoveEntryMessage(UInvSys_InventoryItemInstance* ItemInstance) const;

	void BroadcastAddEntryMessage(UInvSys_InventoryItemInstance* ItemInstance) const;

	FORCEINLINE UWorld* GetWorld() const;

	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE ENetMode GetNetMode() const;

	FORCEINLINE UInvSys_InventoryComponent* GetInventoryComponent() const;

	FORCEINLINE FGameplayTag GetInventoryObjectTag() const;

private:
	template<class C, class V>
	void InitItemInstanceProps(UInvSys_InventoryItemInstance* ItemInstance, const V& Value, bool bIsBroadcast = false)
	{
		if (ItemInstance != nullptr && ItemInstance->IsA<C>())
		{
			((C*)ItemInstance)->InitItemInstanceProps(Value, bIsBroadcast);	// ItemInstance 需要创建对应的函数处理该值
		}
	}
};

template <class T, class ... Arg>
T* FInvSys_ContainerList::AddDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount,
	const Arg&... Args)
{
	check(ItemDef)
	check(InventoryFragment)
	check(InventoryFragment->HasAuthority());
	if (ItemDef == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("AddDefinition Falied, Item definition is nullptr."))
		return nullptr;
	}

	FInvSys_ContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
	T* Result = NewObject<T>(InventoryFragment->GetInventoryComponent());
	// Result->SetInventoryComponent(InventoryFragment->GetInventoryComponent());
	Result->SetItemDefinition(ItemDef);
	Result->SetItemUniqueID(FGuid::NewGuid());
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
	int32 Arr[] = {0, (InitItemInstanceProps<T>(Result, Args, false), 0)...}; 
		
	MarkItemDirty(NewEntry);
		
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		Result->BroadcastAddItemInstanceMessage();
	}
	return Result;
}

template <class T, class ... Arg>
T* FInvSys_ContainerList::AddInstance(UInvSys_InventoryItemInstance* ItemInstance, const Arg&... Args)
{
	check(ItemInstance)
	check(InventoryFragment)
	check(InventoryFragment->HasAuthority());
	if (ItemInstance->IsA<T>() == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("AddInstance Falied, 传入的物品实例的类型不匹配."))
		return nullptr;
	}
	T* TargetItemInstance = Cast<T>(ItemInstance);
	// todo::使用对象池获取新对象，优化深度拷贝对象带来的消耗？
	TargetItemInstance = DuplicateObject<T>(TargetItemInstance, InventoryFragment->GetInventoryComponent());
	ItemInstance->ConditionalBeginDestroy();//标记目标待删除

	// 更新物品的基础信息
	TargetItemInstance->SetSlotTag(InventoryFragment->GetInventoryObjectTag());
	TargetItemInstance->SetIsDraggingItem(false);
	// Instance->SetInventoryComponent(InventoryFragment->GetInventoryComponent());

	//执行可变参数模板，将参数列表中的值赋予目标对象。
	int32 Arr[] = {0, (InitItemInstanceProps<T>(TargetItemInstance, Args, false), 0)...};

	FInvSys_ContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = TargetItemInstance;

	MarkItemDirty(NewEntry);

	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		TargetItemInstance->BroadcastAddItemInstanceMessage();
	}
	return TargetItemInstance;
}

template<>
struct TStructOpsTypeTraits< FInvSys_ContainerList > : public TStructOpsTypeTraitsBase2< FInvSys_ContainerList >
{
	enum { WithNetDeltaSerializer = true };
};
