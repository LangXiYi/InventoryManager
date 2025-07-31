// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InvSys_PickableContainer.h"

#include "Components/InvSys_InventoryComponent.h"


AInvSys_PickableContainer::AInvSys_PickableContainer()
{
}

void AInvSys_PickableContainer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SetupInventoryComponent();
}

void AInvSys_PickableContainer::SetupInventoryComponent()
{
	if (InventoryComponent == nullptr)
	{
		if (InventoryComponentClass == nullptr)
		{
			InventoryComponent = NewObject<UInvSys_InventoryComponent>(this, UInvSys_InventoryComponent::StaticClass(), TEXT("InventoryComponent0"));
			InventoryComponent->RegisterComponent();
		}
		else
		{
			InventoryComponent = NewObject<UInvSys_InventoryComponent>(this, InventoryComponentClass, TEXT("InventoryComponent0"));
			InventoryComponent->RegisterComponent();
		}
	}
	InventoryComponent->InventoryObjectContent = InventoryObjectContent;
	InventoryComponent->LayoutWidgetClass = LayoutWidgetClass;
}
