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
	// Sets default values for this component's properties
	UGridInvSys_GridInventoryControllerComponent();

	/**
	 * 仅适用于同容器组件下的物品转移
	 * @param ChangedItems 旧物品的唯一ID
	 * @param NewItemData 
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void Server_UpdateInventoryItems(const TArray<FName>& ChangedItems, const TArray<FGridInvSys_InventoryItem>& NewItemData);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual UInvSys_InventoryComponent* GetInventoryComponent_Implementation() const override;
};
