// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Equipment.h"
#include "GridInvSys_InventoryFragment_Equipment.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventoryFragment_Equipment : public UInvSys_InventoryFragment_Equipment
{
	GENERATED_BODY()

public:
	virtual UInvSys_InventoryItemInstance* EquipItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount = 1) override;
};
