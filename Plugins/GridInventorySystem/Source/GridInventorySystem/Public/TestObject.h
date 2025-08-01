// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TestObject.generated.h"

USTRUCT()
struct FTestStruct
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<class UTestObjectEntry*> Entries;
};

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UTestObject : public UObject
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	FTestStruct MyStruct;
};
