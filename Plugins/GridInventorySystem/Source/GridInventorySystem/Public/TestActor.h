// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TestObject.h"
#include "GameFramework/Actor.h"
#include "TestActor.generated.h"

UCLASS()
class GRIDINVENTORYSYSTEM_API ATestActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATestActor();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UPROPERTY(Replicated)
	FTestStruct MyStruct;

	UPROPERTY()
	UTestObjectEntry* TestObjectEntryC;
};
