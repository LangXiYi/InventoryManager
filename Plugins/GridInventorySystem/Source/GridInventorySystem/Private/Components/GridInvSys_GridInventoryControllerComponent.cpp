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

void UGridInvSys_GridInventoryControllerComponent::Server_UpdateInventoryItems_Implementation(const TArray<FName>& ChangedItems,
	const TArray<FGridInvSys_InventoryItem>& NewItemData)
{
	if (UGridInvSys_InventoryComponent* GridInvComp = GetInventoryComponent<UGridInvSys_InventoryComponent>())
	{
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

UInvSys_InventoryComponent* UGridInvSys_GridInventoryControllerComponent::GetInventoryComponent_Implementation() const
{
	// todo::本案例的 InventoryComponent 与 ControllerInventoryComponent 都在 Controller下，若位置发送改变需要在这里重写。
	// todo::是否考虑使用Interface？与 GAS 类似的方案？
	return GetOwner()->GetComponentByClass<UGridInvSys_InventoryComponent>();
}

