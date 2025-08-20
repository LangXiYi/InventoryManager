// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/InvSys_ContainerList.h"
#include "Data/InvSys_InventoryItemDefinition.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "InvSys_InventoryModule_Container.generated.h"

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryModule_Container : public UInvSys_InventoryModule
{
	GENERATED_BODY()

public:
	UInvSys_InventoryModule_Container();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Module|Container")
	virtual void RefreshInventoryFragment() override;

	/**
	 * 根据物品定义创建物品并添加至当前容器内
	 * 注意：可变参数列表要求目标类型必须实现 InitItemInstanceProps 函数，且参数类型一致。
	 */
	template<class T, class... ArgList>
	T* AddItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef,
		int32 StackCount, const ArgList&... Args)
	{
		return AddItemDefinition<T>(T::StaticClass(), ItemDef, StackCount, Args...);
	}

	template<class T, class... ArgList>
	T* AddItemDefinition(UClass* ItemInstanceClass, TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef,
		int32 StackCount, const ArgList&... Args)
	{
		T* ItemInstance = ContainerList.AddDefinition<T>(ItemInstanceClass, ItemDef, StackCount, Args...);
		OnContainerPostAdd(ItemInstance);
		MarkItemInstanceDirty(ItemInstance);
		return ItemInstance;
	}

	/**
	 * 添加物品至当前容器
	 * 注意：可变参数列表要求目标类型必须实现 InitItemInstanceProps 函数，且参数类型一致。
	 */
	template<class T, class... ArgList>
	T* AddItemInstance(UInvSys_InventoryItemInstance* ItemInstance, const ArgList&... Args)
	{
		T* NewItemInstance = ContainerList.AddInstance<T>(ItemInstance, Args...);
		OnContainerPostAdd(NewItemInstance);
		MarkItemInstanceDirty(NewItemInstance);
		return NewItemInstance;
	}

	/**
	 * 查找所有可堆叠的物品实例
	 * @param ItemDef 物品类型
	 * @param StackableItems 输出物品实例 
	 * @return 所有物品实例可用的堆叠数量
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Module|Container")
	int32 FindStackableItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef,
	                                 TArray<UInvSys_InventoryItemInstance*>& StackableItems);

	/**
	 * 查找所有可堆叠的物品实例
	 * @param ItemInstance 物品类型
	 * @param StackableItems 输出物品实例 
	 * @return 所有物品实例可用的堆叠数量
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Module|Container")
	int32 FindStackableItemInstances(UInvSys_InventoryItemInstance* ItemInstance,
									 TArray<UInvSys_InventoryItemInstance*>& StackableItems);

	/**
	 * 更新容器内物品的属性
	 * 注意：可变参数列表要求目标类型必须实现 InitItemInstanceProps 函数，且参数类型一致。
	 */
	template<class T, class... ArgList>
	void UpdateItemInstance(UInvSys_InventoryItemInstance* ItemInstance, const ArgList&... Args)
	{
		check(ItemInstance)
		if (ContainsItem(ItemInstance))
		{
			//执行可变参数模板，将参数列表中的值赋予目标对象。
			int32 Arr[] = {0, (InitItemInstanceProps<T>(ItemInstance, Args, true), 0)...};
			MarkItemInstanceDirty(ItemInstance);
		}
	}

	/** 移除所有物品 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Module|Container")
	void RemoveAllItemInstance();

	/** 移除指定物品 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Module|Container")
	bool RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	/** 获取当前容器内的所有物品 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Module|Container")
	bool ContainsItem(UInvSys_InventoryItemInstance* ItemInstance) const;

	/** 获取当前容器内的所有物品 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Module|Container")
	void GetAllItemInstance(TArray<UInvSys_InventoryItemInstance*>& OutArray) const;

	/** 标记指定物品为脏 */
	void MarkItemInstanceDirty(UInvSys_InventoryItemInstance* ItemInstance);

	virtual void MarkInventoryModuleDirty() override;

protected:
	virtual void OnContainerPostAdd(UInvSys_InventoryItemInstance* ItemInstance) {}
	virtual void OnContainerPreRemove(UInvSys_InventoryItemInstance* ItemInstance) {}

	/**
	 * 该函数参考 ActorChannel 的 KeyNeedsToReplicate，使用映射记录不同元素的脏标记
	 * 一个简单的脏标记记录，检查映射的键值对与传入的数据是否一致，若不一致则表明数据已经被修改，并返回 true
	 */
	bool KeyNeedsToReplicate(int32 ObjID, int32 RepKey);

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
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

	TMap<int32, int32> ContainerEntryRepKeyMap;

private:
	FGameplayMessageListenerHandle OnItemStackChangedHandle;
	FGameplayMessageListenerHandle OnAddItemInstanceHandle;
	FGameplayMessageListenerHandle OnRemoveItemInstanceHandle;
};
