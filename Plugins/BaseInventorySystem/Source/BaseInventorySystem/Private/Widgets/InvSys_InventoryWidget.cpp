// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryWidget.h"

void UInvSys_InventoryWidget::SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp)
{
	InventoryComponent = NewInvComp;
}

UInvSys_InventoryComponent* UInvSys_InventoryWidget::GetInventoryComponent() const
{
	return InventoryComponent;
}


