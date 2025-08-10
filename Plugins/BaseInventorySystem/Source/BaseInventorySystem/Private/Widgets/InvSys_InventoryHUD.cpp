// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryHUD.h"

#include "BaseInventorySystem.h"
#include "Widgets/Components/InvSys_InventoryItemActionPanel.h"

void UInvSys_InventoryHUD::DisplayInventoryItemActionList(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}
	if (ItemActionPanel == nullptr)
	{
		ItemActionPanel = CreateWidget<UInvSys_InventoryItemActionPanel>(this, ItemActionPanelClass);
		ItemActionPanel->AddToViewport();
	}
	check(ItemActionPanel);
	ItemActionPanel->NativeOnCallOut(ItemInstance);
}
