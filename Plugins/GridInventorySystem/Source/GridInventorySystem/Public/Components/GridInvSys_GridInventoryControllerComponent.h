// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InvSys_InventoryControllerComponent.h"
#include "GridInvSys_GridInventoryControllerComponent.generated.h"


UCLASS(ClassGroup=(InventorySystem), meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYSYSTEM_API UGridInvSys_GridInventoryControllerComponent : public UInvSys_InventoryControllerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGridInvSys_GridInventoryControllerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
