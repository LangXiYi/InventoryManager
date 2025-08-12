// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/GridInvSys_PickableContainer.h"

#include "Components/InvSys_InventoryComponent.h"


// Sets default values
AGridInvSys_PickableContainer::AGridInvSys_PickableContainer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGridInvSys_PickableContainer::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGridInvSys_PickableContainer::SetupInventoryComponent()
{
	if (InventoryComponent == nullptr)
	{
		InventoryComponent = NewObject<UInvSys_InventoryComponent>(this, InventoryComponentClass, TEXT("InventoryComponent0"));
		InventoryComponent->RegisterInventoryComponent(InventoryObjectContent, LayoutWidgetClass);
	}
}

// Called every frame
void AGridInvSys_PickableContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

