// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InventoryObject/GridInvSys_GridEquipContainerObject.h"
#include "Data/GridInvSys_ItemFragment_ItemType.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Widgets/InvSys_EquipContainerSlotWidget.h"
#include "GridInvSys_EquipContainerSlotWidget.generated.h"

struct FGridInvSys_ItemPosition;
class UInvSys_InventoryItemInstance;
class UGridInvSys_ContainerGridItemWidget;

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_EquipContainerSlotWidget : public UInvSys_EquipContainerSlotWidget
{
	GENERATED_BODY()

public:
	/*UGridInvSys_ContainerGridItemWidget* FindGridItemWidget(const FGridInvSys_ItemPosition& ItemPosition) const;

	UGridInvSys_ContainerGridItemWidget* FindGridItemWidget(const UInvSys_InventoryItemInstance* NewItemInstance) const;*/

	UFUNCTION(BlueprintCallable, Category = "Grid Container")
	void AddItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Grid Container")
	void AddItemInstanceTo(UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPosition);

	UFUNCTION(BlueprintCallable, Category = "Grid Container")
	void RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Grid Container")
	void RemoveItemInstanceFor(UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPosition);

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

private:
	FGameplayMessageListenerHandle OnAddItemInstanceHandle;
	FGameplayMessageListenerHandle OnRemoveItemInstanceHandle;
	
	void OnAddItemInstance(FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message);

	void OnRemoveItemInstance(FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message);
};
