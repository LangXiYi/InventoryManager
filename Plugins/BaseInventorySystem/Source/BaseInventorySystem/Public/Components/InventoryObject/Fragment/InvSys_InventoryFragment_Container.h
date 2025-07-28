// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	virtual void InitInventoryFragment(/*UInvSys_BaseInventoryObject* InvObj, */UObject* PreEditFragment) override;

	virtual void RefreshInventoryFragment() override;
	
	/**
	 * 主要用于初始化库存，或不同库存组件间的物品交换
	 * 注意：传入可变参数时，请确保目标类型中正确创建了对于的处理函数。
	 */
	template<class T = UInvSys_InventoryItemInstance, class... Arg>
	T* AddItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount, const Arg&... Args)
	{
		check(ItemDef)
		return ContainerList.AddEntry<T>(ItemDef, StackCount, Args...);
	}

	/** 从其他容器添加物品，容器与容器间的交换，不会 RemoveReplicateObject，因为它们都在同一个Actor下 */
	template<class T = UInvSys_InventoryItemInstance, class... Arg>
	bool AddItemInstance(T* ItemInstance, const Arg&... Args)
	{
		check(ItemInstance)
		return ContainerList.AddEntry(ItemInstance, Args...);
	}

	virtual void RemoveAllItemInstance();

	virtual bool RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	virtual bool UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance, int32 NewStackCount);

	virtual bool ContainsItem(UInvSys_InventoryItemInstance* ItemInstance) const;

	TArray<UInvSys_InventoryItemInstance*> GetAllItemInstance();

protected:
	virtual void NativeOnInventoryStackChange(FInvSys_InventoryStackChangeMessage ChangeInfo) {}
	virtual void NativeOnContainerEntryAdded(FInvSys_InventoryItemChangedMessage ChangeInfo) {}
	virtual void NativeOnContainerEntryRemove(FInvSys_InventoryItemChangedMessage ChangeInfo) {}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	UPROPERTY(Replicated)
	FInvSys_ContainerList ContainerList;

private:
	FGameplayMessageListenerHandle OnAddItemInstanceHandle;
	FGameplayMessageListenerHandle OnRemoveItemInstanceHandle;
};
