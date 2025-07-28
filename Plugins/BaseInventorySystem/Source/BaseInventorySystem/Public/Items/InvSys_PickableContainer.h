// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_PickableItems.h"
#include "InvSys_PickableContainer.generated.h"

class UInvSys_InventoryComponent;

UCLASS()
class BASEINVENTORYSYSTEM_API AInvSys_PickableContainer : public AInvSys_PickableItems
{
	GENERATED_BODY()

public:
	AInvSys_PickableContainer();

	// todo::add inventory component 子类有可能会重载它。

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Pickable Container")
	TObjectPtr<UInvSys_InventoryComponent> InventoryComponent;
};
