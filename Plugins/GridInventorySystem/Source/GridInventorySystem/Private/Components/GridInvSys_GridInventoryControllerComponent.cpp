// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_GridInventoryControllerComponent.h"

#include "Components/GridInvSys_InventoryComponent.h"


// Sets default values for this component's properties
UGridInvSys_GridInventoryControllerComponent::UGridInvSys_GridInventoryControllerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UGridInvSys_GridInventoryControllerComponent::Server_UpdateInventoryItems_Implementation(
	UInvSys_InventoryComponent* TargetInvComp, const TArray<FName>& ChangedItems,
	const TArray<FGridInvSys_InventoryItemPosition>& NewItemData)
{
	if (TargetInvComp && TargetInvComp->IsA(UGridInvSys_InventoryComponent::StaticClass()))
	{
		UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(TargetInvComp);
		GridInvComp->UpdateContainerItemsPosition(ChangedItems, NewItemData);
	}
}

// Called when the game starts
void UGridInvSys_GridInventoryControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGridInvSys_GridInventoryControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
