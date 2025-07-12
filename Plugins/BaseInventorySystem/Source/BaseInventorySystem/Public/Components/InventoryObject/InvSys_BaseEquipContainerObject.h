// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_BaseEquipmentObject.h"
#include "Data/InvSys_ContainerList.h"
#include "InvSys_BaseEquipContainerObject.generated.h"


class UInvSys_InventoryWidget;
class UInvSys_ContainerLayoutWidget;
class UInvSys_InventoryItemDefinition;
class UInvSys_InventoryItemInstance;

/**
 *
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_BaseEquipContainerObject : public UInvSys_BaseEquipmentObject
{
	GENERATED_BODY()

	friend UInvSys_InventoryComponent;

	// 如果是添加Item，则为ItemInstance添加代理，监听属性变化
	// 如果是Remove，则移除所有相关的代理
	// 监听到属性变化时，比如位置发生改变，根据旧位置以及新位置，更新控件
	// 如果是数量发生改变？通知控件更新显示效果。

	// 添加新物品时，设置监听，监听物品的属性变化
	// 当移动物品至另一容器时，移除监听
	// 如果是不同的库存组件进行移动
	// 把这个物品在旧库存组件中删除，然后在新库存组件中创建新的物品？
	// 或者是把物品在旧库存中删除，在新库存组件中，将物品实例的Owner转移到新库存组件中。
	
public:
	UInvSys_BaseEquipContainerObject();

	virtual void OnConstructInventoryObject(UInvSys_InventoryComponent* NewInvComp, UObject* PreEditPayLoad) override;

	virtual UInvSys_EquipSlotWidget* CreateDisplayWidget(APlayerController* PC) override;

	virtual void TryRefreshEquipSlot(const FString& Reason) override;
	virtual void TryRefreshContainerItems();

	/**
	 * 主要用于初始化库存，或不同库存组件间的物品交换
	 * 注意：传入可变参数时，请确保目标类型中正确创建了对于的处理函数。
	 */
	template<class T, class... Arg>
	T* AddItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount, const Arg&... Args)
	{
		if (ItemDef == nullptr) return nullptr;
		T* ItemInstance = ContainerList.AddEntry<T>(ItemDef, StackCount, Args...);
		return ItemInstance;
	}

	/** 从其他容器添加物品，容器与容器间的交换，不会 RemoveReplicateObject，因为它们都在同一个Actor下 */
	bool AddItemInstance(UInvSys_InventoryItemInstance* ItemInstance);
	bool AddItemInstances(TArray<UInvSys_InventoryItemInstance*> ItemInstances);
	virtual bool RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance) override;
	virtual bool RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance) override;
	bool UpdateItemStackCount(UInvSys_InventoryItemInstance* ItemInstance, int32 NewStackCount);

protected:
	virtual void NativeOnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance) override;

	virtual void NativeOnUnEquipItemInstance() override;
	
	// Begin Listen Container List Event ====
	void NativeOnInventoryStackChange(FInvSys_InventoryStackChangeMessage ChangeInfo);
	void NativeOnContainerEntryAdded(const FInvSys_ContainerEntry& Entry, bool bIsForceRep);
	void NativeOnContainerEntryRemove(const FInvSys_ContainerEntry& Entry, bool bIsForceRep);

	virtual void OnInventoryStackChange(const FInvSys_InventoryStackChangeMessage& ChangeInfo) {}
	virtual void OnContainerEntryAdded(const FInvSys_ContainerEntry& Entry, bool bIsForceRep) {}
	virtual void OnContainerEntryRemove(const FInvSys_ContainerEntry& Entry, bool bIsForceRep) {}
	// End Listen Container List Event ====

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

public:
	/**
	 * Getter Or Setter
	 **/

	virtual bool ContainsItem(FGuid ItemUniqueID) override;

	virtual void CopyPropertyFromPreEdit(UObject* PreEditPayLoad) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const UInvSys_InventoryItemInstance* FindItemInstance(FGuid ItemUniqueID) const;

protected:
	UPROPERTY(Replicated)
	FInvSys_ContainerList ContainerList;

	UPROPERTY()
	TObjectPtr<UInvSys_InventoryWidget> ContainerLayout;

	TMap<FGuid, FInvSys_ContainerEntry> ContainerEntryMap;
};
