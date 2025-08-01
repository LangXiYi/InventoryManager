// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TestObjectEntry.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UTestObjectEntry : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_Name)
	FString Name = "Test Object";

	UFUNCTION()
	void OnRep_Name();
};
