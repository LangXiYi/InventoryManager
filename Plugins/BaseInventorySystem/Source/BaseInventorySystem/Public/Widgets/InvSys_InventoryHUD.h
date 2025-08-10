// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvSys_InventoryHUD.generated.h"

class UInvSys_InventoryItemInstance;
class UInvSys_InventoryItemActionPanel;
/**
 * 供库存系统使用，可与其他系统的HUD搭配使用
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory HUD")
	void DisplayInventoryItemActionList(UInvSys_InventoryItemInstance* ItemInstance);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory HUD")
	TObjectPtr<UInvSys_InventoryItemActionPanel> ItemActionPanel;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory HUD")
	TSubclassOf<UInvSys_InventoryItemActionPanel> ItemActionPanelClass;

};
