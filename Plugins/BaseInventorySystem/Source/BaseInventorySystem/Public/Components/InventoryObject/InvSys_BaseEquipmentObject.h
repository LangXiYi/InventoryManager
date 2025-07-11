// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_BaseInventoryObject.h"
#include "InvSys_CommonType.h"
#include "InvSys_BaseEquipmentObject.generated.h"

class UInvSys_EquipSlotWidget;
class UInvSys_InventoryItemDefinition;
class UInvSys_InventoryItemInstance;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_BaseEquipmentObject : public UInvSys_BaseInventoryObject
{
	GENERATED_BODY()

public:
	UInvSys_BaseEquipmentObject();
	
	virtual void AddInventoryItemToEquipSlot_DEPRECATED(const FInvSys_InventoryItem& NewItem);

	virtual UInvSys_InventoryItemInstance* EquipItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef);

	virtual void EquipInventoryItem(UInvSys_InventoryItemInstance* NewItemInstance);

	virtual void UnEquipInventoryItem();

	virtual bool RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance) override;

	virtual bool RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance) override;

	virtual UInvSys_EquipSlotWidget* CreateDisplayWidget(APlayerController* PC) override;

protected:
	/**	刷新显示效果 */
	virtual void TryRefreshEquipSlot(const FString& Reason = "");

	virtual void CopyPropertyFromPreEdit(UObject* PreEditPayLoad) override;

	virtual void NativeOnEquipItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	virtual void NativeOnUnEquipItemInstance();

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
public:
	/**
	 * Getter Or Setter
	 **/

	virtual bool ContainsItem(FName UniqueID) override;

	UInvSys_InventoryItemInstance* GetEquipItemInstance() const
	{
		return EquipItem;
	}

	/*bool IsEquipped() const
	{
		return false;
	}*/

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// 装备的物品
	UPROPERTY(ReplicatedUsing=OnRep_EquipItemInstance, BlueprintReadOnly, Category = "Inventory Object")
	TObjectPtr<UInvSys_InventoryItemInstance> EquipItem;
	UFUNCTION()
	virtual void OnRep_EquipItemInstance();
	// [Client] 标记上一次装备的物品
	UPROPERTY(Transient)
	TWeakObjectPtr<UInvSys_InventoryItemInstance> LastEquipItemInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Container Type")
	TObjectPtr<UInvSys_EquipSlotWidget> EquipSlotWidget;
	/** 装备控件类型 */
	UPROPERTY(BlueprintReadOnly, Category = "Container Type")
	TSubclassOf<UInvSys_EquipSlotWidget> EquipSlotWidgetClass;
};

/**
 * Pre Edit Inventory Object
 */

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_PreEditEquipmentObject : public UInvSys_PreEditInventoryObject
{
	GENERATED_BODY()

public:
	/** 容器构建函数 */
	CONSTRUCT_INVENTORY_OBJECT(UInvSys_BaseEquipmentObject);

	
	/** 装备控件类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Type")
	TSubclassOf<UInvSys_EquipSlotWidget> EquipSlotWidgetClass;
};