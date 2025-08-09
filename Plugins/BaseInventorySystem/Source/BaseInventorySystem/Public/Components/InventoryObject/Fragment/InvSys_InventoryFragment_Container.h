// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/InvSys_ContainerList.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "InvSys_InventoryFragment_Container.generated.h"

/**
 * 如果添加了可装备片段，那么容器片段中的容器布局就应该是可变的
 * 如果没有可装备片段，那么容器片段中的容器布局就应该是不可变的
 * 所以这个构建库存容器的作用是什么？
 * 在库存对象中，存储了该对象的所有的片段，所以它应该知道
 * 库存对象在初始化片段时，检查是否存在可装备片段，如果
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryFragment_Container : public UInvSys_BaseInventoryFragment
{
	GENERATED_BODY()

public:
	UInvSys_InventoryFragment_Container();

	virtual void InitInventoryFragment(UObject* PreEditFragment) override;

	virtual void RefreshInventoryFragment() override;

	/**
	 * 主要用于初始化库存，或不同库存组件间的物品交换
	 * 注意：传入可变参数时，请确保目标类型中正确创建了对于的处理函数。
	 */
	template<class T, class... Arg>
	T* AddItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount, const Arg&... Args)
	{
		check(ItemDef)
		T* ItemInstance = ContainerList.AddDefinition<T>(ItemDef, StackCount, Args...);
		if (ItemInstance)
		{
			MarkItemInstanceDirty(ItemInstance);
		}
		return ItemInstance;
	}

	// 作为对象属性，ContainerList 的复制优先 其内部其他对象的 复制！！！
	/** 从其他容器添加物品，容器与容器间的交换，不会 RemoveReplicateObject，因为它们都在同一个Actor下 */
	template<class T, class... Arg>
	T* AddItemInstance(UInvSys_InventoryItemInstance* ItemInstance, const Arg&... Args)
	{
		T* NewItemInstance = nullptr;
		if (ItemInstance)
		{
			NewItemInstance = ContainerList.AddInstance<T>(ItemInstance, Args...);
			MarkItemInstanceDirty(NewItemInstance);
		}
		return NewItemInstance;
	}

	template<class T, class... Arg>
	bool UpdateItemInstance(UInvSys_InventoryItemInstance* ItemInstance, const Arg&... Args)
	{
		if (ItemInstance == nullptr) return false;
		//执行可变参数模板，将参数列表中的值赋予目标对象。
		int32 Arr[] = {0, (InitItemInstanceProps<T>(ItemInstance, Args, true), 0)...};
		MarkItemInstanceDirty(ItemInstance);
		return true;
	}

	bool UpdateItemInstanceDragState(UInvSys_InventoryItemInstance* ItemInstance, bool NewState);

	virtual void RemoveAllItemInstance();

	virtual bool RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	virtual bool UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance, int32 NewStackCount);

	virtual bool ContainsItem(UInvSys_InventoryItemInstance* ItemInstance) const;

	UFUNCTION(BlueprintCallable)
	void GetAllItemInstance(TArray<UInvSys_InventoryItemInstance*>& OutArray);

	void MarkItemInstanceDirty(UInvSys_InventoryItemInstance* ItemInstance);

	void MarkContainerDirty();

protected:
	virtual void NativeOnItemStackChange(FInvSys_InventoryStackChangeMessage ChangeInfo) {} // DEPRECATED
	virtual void NativeOnContainerEntryAdded(FInvSys_InventoryItemChangedMessage ChangeInfo) {}
	virtual void NativeOnContainerEntryRemove(FInvSys_InventoryItemChangedMessage ChangeInfo) {}

	/**
	 * 该函数参考 ActorChannel 的 KeyNeedsToReplicate，使用映射记录不同元素的脏标记
	 * 一个简单的脏标记记录，检查映射的键值对与传入的数据是否一致，若不一致则表明数据已经被修改，并返回 true
	 */
	bool KeyNeedsToReplicate(int32 ObjID, int32 RepKey);

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void Debug_PrintContainerAllItems();

	template<class C, class V>
	void InitItemInstanceProps(UInvSys_InventoryItemInstance* ItemInstance, const V& Value, bool bIsBroadcast = true)
	{
		if (ItemInstance != nullptr && ItemInstance->IsA<C>())
		{
			((C*)ItemInstance)->InitItemInstanceProps(Value, bIsBroadcast);	// ItemInstance 需要创建对应的函数处理该值
		}
	}

protected:
	UPROPERTY(Replicated)
	FInvSys_ContainerList ContainerList;
	// UFUNCTION()
	// void OnRep_ContainerList();

	TMap<int32, int32> ContainerEntryRepKeyMap;

private:
	FGameplayMessageListenerHandle OnItemStackChangedHandle;
	FGameplayMessageListenerHandle OnAddItemInstanceHandle;
	FGameplayMessageListenerHandle OnRemoveItemInstanceHandle;
};
