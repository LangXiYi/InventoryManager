// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "GridInvSys_EquipmentSlotWidget.generated.h"

class UInvSys_InventoryItemInstance;
class UGridInvSys_DragDropWidget;
class UGridInvSys_ContainerGridLayoutWidget;
struct FInvSys_InventoryItem;

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_EquipmentSlotWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	virtual void RefreshInventoryWidget(UInvSys_BaseInventoryObject* NewInventoryObject) override;
	
	virtual void EquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);
	virtual void UnEquipItemInstance();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
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

protected:
	UPROPERTY()
	TWeakObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;

	FGameplayMessageListenerHandle OnEquipItemInstanceHandle;
	FGameplayMessageListenerHandle OnUnEquipItemInstanceHandle;
};
