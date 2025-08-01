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
	void Server_RestoreItemInstanceToPos(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryDropItemInstance(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SwapItemInstance(UInvSys_InventoryItemInstance* FromItemInstance,
		UInvSys_InventoryItemInstance* ToItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_UpdateItemInstancePosition(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* ItemInstance, FGridInvSys_ItemPosition NewPosition);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TestFunc(UInvSys_InventoryComponent* InvComp,
		const TArray<class UGridInvSys_InventoryItemInstance*>& Array, const TArray<FGridInvSys_ItemPosition>& NewItemPositions,
		UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& DropPosition);
};
