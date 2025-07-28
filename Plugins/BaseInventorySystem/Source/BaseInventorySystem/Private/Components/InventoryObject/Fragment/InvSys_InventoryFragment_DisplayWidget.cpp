// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_DisplayWidget.h"

#include "BaseInventorySystem.h"
#include "Widgets/InvSys_InventoryWidget.h"

UInvSys_InventoryFragment_DisplayWidget::UInvSys_InventoryFragment_DisplayWidget()
{
	Priority = 0;
}

void UInvSys_InventoryFragment_DisplayWidget::InitInventoryFragment(UObject* PreEditFragment)
{
	Super::InitInventoryFragment(PreEditFragment);
	COPY_INVENTORY_FRAGMENT_PROPERTY(UInvSys_InventoryFragment_DisplayWidget, DisplayWidgetClass);
}

void UInvSys_InventoryFragment_DisplayWidget::RefreshInventoryFragment()
{
	Super::RefreshInventoryFragment();
	check(DisplayWidget)
	DisplayWidget->RefreshWidget();
}

UInvSys_InventoryWidget* UInvSys_InventoryFragment_DisplayWidget::CreateDisplayWidget(APlayerController* PC)
{
	if (DisplayWidget == nullptr)
	{
		if (PC != nullptr && PC->IsLocalController())
		{
			DisplayWidget = CreateWidget<UInvSys_InventoryWidget>(PC, DisplayWidgetClass);
			DisplayWidget->SetInventoryObject(GetInventoryObject());
		}
	}
	check(DisplayWidget);
	return DisplayWidget;
}
