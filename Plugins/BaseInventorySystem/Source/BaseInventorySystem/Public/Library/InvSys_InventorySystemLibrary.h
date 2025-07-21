// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvSys_InventorySystemLibrary.generated.h"

class UInvSys_InventoryControllerComponent;
class IInvSys_InventoryInterface;
class UInvSys_InventoryComponent;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventorySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory System Library")
	static UInvSys_InventoryComponent* FindInventoryComponent(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "Inventory System Library")
	static UInvSys_InventoryControllerComponent* FindInvControllerComponent(const UWorld* World);

	template<class T>
	static T* FindInvControllerComponent(const UWorld* World)
	{
		return (T*)FindInvControllerComponent(World);
	}
};
