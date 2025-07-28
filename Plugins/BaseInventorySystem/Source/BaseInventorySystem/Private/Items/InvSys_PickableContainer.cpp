// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InvSys_PickableContainer.h"

#include "Components/InvSys_InventoryComponent.h"


AInvSys_PickableContainer::AInvSys_PickableContainer()
{
	InventoryComponent = CreateDefaultSubobject<UInvSys_InventoryComponent>("Inventory Component");
}
