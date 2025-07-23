// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InvSys_StorageBin.generated.h"

UCLASS()
class BASEINVENTORYSYSTEM_API AInvSys_StorageBin : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInvSys_StorageBin();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
