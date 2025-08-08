// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/InvSys_InventoryControllerComponent.h"
#include "GridInvSys_GridInventoryControllerComponent.generated.h"


class UGridInvSys_InventoryItemInstance;

UCLASS(ClassGroup=(InventorySystem), meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYSYSTEM_API UGridInvSys_GridInventoryControllerComponent : public UInvSys_InventoryControllerComponent
{
	GENERATED_BODY()

public:
	UGridInvSys_GridInventoryControllerComponent();

	/**
	 * Local Prediction Function
	 **/

	UFUNCTION(BlueprintCallable)
	void TryDropItemInstance(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);

	/**
	 * RPC Function
	 **/

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_AddItemInstancesToContainerPos(UInvSys_InventoryComponent* InvComp,
		const TArray<UInvSys_InventoryItemInstance*>& InItemInstances, const TArray<FGridInvSys_ItemPosition>& InPosArray);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_RemoveItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_CancelOccupied(UInvSys_InventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_RestoreItemInstanceToPos(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryDropItemInstance(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SwapItemInstance(UGridInvSys_InventoryItemInstance* FromItemInstance, UGridInvSys_InventoryItemInstance* ToItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SwapItemInstances(UInvSys_InventoryComponent* ToInvComp,
		UGridInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& ToPosition,
		const TArray<UGridInvSys_InventoryItemInstance*>& TargetItemInstances, const TArray<FGridInvSys_ItemPosition>& TargetPositions);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_UpdateItemInstancePosition(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* ItemInstance, FGridInvSys_ItemPosition NewPosition);
};
