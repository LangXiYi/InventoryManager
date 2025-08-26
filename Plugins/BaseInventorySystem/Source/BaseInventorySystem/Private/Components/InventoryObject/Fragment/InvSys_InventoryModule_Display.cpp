// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_InventoryModule_Display.h"

#include "BaseInventorySystem.h"
#include "Library/InvSys_InventorySystemLibrary.h"
#include "Widgets/InvSys_InventoryHUD.h"
#include "Widgets/InvSys_InventoryWidget.h"

UInvSys_InventoryModule_Display::UInvSys_InventoryModule_Display()
{
	
}

void UInvSys_InventoryModule_Display::InitInventoryFragment(UObject* PreEditFragment)
{
	Super::InitInventoryFragment(PreEditFragment);
	COPY_INVENTORY_FRAGMENT_PROPERTY(UInvSys_InventoryModule_Display, DisplayWidgetClass);
	COPY_INVENTORY_FRAGMENT_PROPERTY(UInvSys_InventoryModule_Display, WidgetActivity);
}

void UInvSys_InventoryModule_Display::RefreshInventoryFragment()
{
	Super::RefreshInventoryFragment();
	// DisplayWidget->RefreshWidget();
	if (DisplayWidget.IsValid())
	{
		DisplayWidget->InitInventoryWidget(InventoryObject);
	}
}

UInvSys_InventoryWidget* UInvSys_InventoryModule_Display::TryCreateDisplayWidget(APlayerController* PC)
{
	if (DisplayWidget.IsValid() && DisplayWidget->HasAnyFlags(EObjectFlags::RF_BeginDestroyed) == false)
	{
		return DisplayWidget.Get();
	}

	if (PC && PC->IsLocalController())
	{
		UInvSys_InventoryHUD* InventoryHUD = UInvSys_InventorySystemLibrary::GetInventoryHUD(GetWorld());
		if (InventoryHUD)
		{
			DisplayWidget = CreateWidget<UInvSys_InventoryWidget>(PC, DisplayWidgetClass);
			DisplayWidget->InitInventoryWidget(InventoryObject);
			InventoryHUD->AddWidget(DisplayWidget.Get(), GetInventoryTag());
		}
	}
	return DisplayWidget.Get();
}

UInvSys_InventoryWidget* UInvSys_InventoryModule_Display::GetDisplayWidget() const
{
	if (DisplayWidget.IsValid())
	{
		return DisplayWidget.Get();
	}
	return nullptr;
}
