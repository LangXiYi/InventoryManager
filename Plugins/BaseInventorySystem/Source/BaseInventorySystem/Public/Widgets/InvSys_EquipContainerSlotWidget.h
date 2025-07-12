// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_EquipSlotWidget.h"
#include "InvSys_EquipContainerSlotWidget.generated.h"

class UInvSys_ContainerLayoutWidget;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_EquipContainerSlotWidget : public UInvSys_EquipSlotWidget
{
	GENERATED_BODY()

public:
	// void AddContainerLayout(TObjectPtr<UInvSys_InventoryWidget> ContainerLayout);

	UFUNCTION(BlueprintImplementableEvent)
	UInvSys_InventoryWidget* GetContainerLayoutWidget();

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Equip Container")
	TObjectPtr<UInvSys_InventoryWidget> ContainerLayoutWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UNamedSlot> NS_ContainerGridLayout;
};