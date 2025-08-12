// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/InvSys_PickableItems.h"
#include "GridInvSys_PickableItems.generated.h"

UCLASS()
class GRIDINVENTORYSYSTEM_API AGridInvSys_PickableItems : public AInvSys_PickableItems
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGridInvSys_PickableItems();

public:
	virtual bool PickupItem(UInvSys_InventoryComponent* InvComp) override;
};
