// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/InvSys_InventoryControllerComponent.h"
#include "GridInvSys_GridInventoryControllerComponent.generated.h"


UCLASS(ClassGroup=(InventorySystem), meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYSYSTEM_API UGridInvSys_GridInventoryControllerComponent : public UInvSys_InventoryControllerComponent
{
	GENERATED_BODY()

public:
	UGridInvSys_GridInventoryControllerComponent();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryDropItemInstance(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SwapItemInstance(UInvSys_InventoryItemInstance* FromItemInstance,
		UInvSys_InventoryItemInstance* ToItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_UpdateItemInstancePosition(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* ItemInstance, FGridInvSys_ItemPosition NewPosition);
};
