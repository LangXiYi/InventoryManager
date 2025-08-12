// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/InvSys_PickableContainer.h"
#include "GridInvSys_PickableContainer.generated.h"

UCLASS()
class GRIDINVENTORYSYSTEM_API AGridInvSys_PickableContainer : public AInvSys_PickableContainer
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGridInvSys_PickableContainer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SetupInventoryComponent() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
