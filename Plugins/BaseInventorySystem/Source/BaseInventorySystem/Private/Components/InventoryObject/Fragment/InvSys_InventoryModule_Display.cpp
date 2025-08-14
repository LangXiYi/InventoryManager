// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_InventoryModule_Display.h"

#include "BaseInventorySystem.h"
#include "Widgets/InvSys_InventoryWidget.h"

UInvSys_InventoryModule_Display::UInvSys_InventoryModule_Display()
{
	Priority = 0;
}

void UInvSys_InventoryModule_Display::InitInventoryFragment(UObject* PreEditFragment)
{
	Super::InitInventoryFragment(PreEditFragment);
	COPY_INVENTORY_FRAGMENT_PROPERTY(UInvSys_InventoryModule_Display, DisplayWidgetClass);
}

void UInvSys_InventoryModule_Display::RefreshInventoryFragment()
{
	Super::RefreshInventoryFragment();
	check(DisplayWidget)
	// DisplayWidget->RefreshWidget();
	if (DisplayWidget)
	{
		DisplayWidget->RefreshInventoryWidget(InventoryObject);
	}
}

UInvSys_InventoryWidget* UInvSys_InventoryModule_Display::CreateDisplayWidget(APlayerController* PC)
{
	if (DisplayWidget == nullptr)
	{
		if (PC != nullptr && PC->IsLocalController())
		{
			DisplayWidget = CreateWidget<UInvSys_InventoryWidget>(PC, DisplayWidgetClass);
			DisplayWidget->RefreshInventoryWidget(InventoryObject);
		}
	}
	check(DisplayWidget);
	return DisplayWidget;
}
