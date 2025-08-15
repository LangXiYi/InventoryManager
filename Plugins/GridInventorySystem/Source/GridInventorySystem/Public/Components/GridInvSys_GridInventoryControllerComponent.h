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

public:
	/**
	 * RPC Function
	 **/

	virtual void Server_UnEquipItemInstance_Implementation(UInvSys_InventoryItemInstance* ItemInstance) override;

	virtual void Server_SplitItemInstance_Implementation(UInvSys_InventoryItemInstance* ItemInstance, int32 SplitSize) override;

	virtual void Server_SwapItemInstance_Implementation(UInvSys_InventoryItemInstance* FromItemInstance, UInvSys_InventoryItemInstance* ToItemInstance) override;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_AddItemInstancesToContainerPos(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance, FGridInvSys_ItemPosition InPos);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_CancelOccupied(UInvSys_InventoryItemInstance* ItemInstance);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryDropItemInstance(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SwapItemInstances(UInvSys_InventoryComponent* ToInvComp,
		UGridInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& ToPosition,
		const TArray<UGridInvSys_InventoryItemInstance*>& TargetItemInstances, const TArray<FGridInvSys_ItemPosition>& TargetPositions);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_UpdateItemInstancePosition(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* ItemInstance, FGridInvSys_ItemPosition NewPosition);
};
