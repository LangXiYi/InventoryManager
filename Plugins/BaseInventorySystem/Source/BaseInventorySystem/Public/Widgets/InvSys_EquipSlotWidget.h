// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryWidget.h"
#include "InvSys_EquipSlotWidget.generated.h"

class UInvSys_InventoryItemInstance;
class UInvSys_BaseInventoryObject;
class UInvSys_BaseEquipmentObject;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_EquipSlotWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	virtual void EquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);
	virtual void UnEquipItemInstance();
	
	void SetInventoryObject(UInvSys_BaseEquipmentObject* NewInvObj);

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:
	/**
	 * 创建 InventoryItemWidget 
	 * @param NewItemInstance 
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnEquipInventoryItem(UInvSys_InventoryItemInstance* NewItemInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUnEquipInventoryItem();

	UFUNCTION(BlueprintNativeEvent)
	bool CheckIsCanDrop(UInvSys_InventoryItemInstance* InItemInstance); // DEPRECATED
	
protected:
	UPROPERTY()
	TWeakObjectPtr<UInvSys_BaseInventoryObject> InvObj;

	UPROPERTY()
	TWeakObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;
};
