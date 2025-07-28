// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryWidget.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

#include "Blueprint/WidgetTree.h"
#include "Components/InvSys_InventoryComponent.h"

UInvSys_InventoryWidget::UInvSys_InventoryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UInvSys_InventoryWidget::RefreshWidget()
{
	OnRefreshWidget();
}

void UInvSys_InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// checkf(InventoryObject, TEXT("You must set inventory object for inventory widget."));
}

void UInvSys_InventoryWidget::SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp)
{
	InventoryComponent = NewInvComp;
}

UInvSys_InventoryComponent* UInvSys_InventoryWidget::GetInventoryComponent() const
{
	return InventoryComponent.Get();
}

void UInvSys_InventoryWidget::SetInventoryObject(UInvSys_BaseInventoryObject* NewInventoryObject)
{
	InventoryObject = NewInventoryObject;
	if (InventoryObject)
	{
		SlotTag = InventoryObject->GetInventoryObjectTag();
		InventoryComponent = InventoryObject->GetInventoryComponent();
	}
}

