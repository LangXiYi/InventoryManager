// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InvSys_InventoryControllerComponent.h"
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
	UFUNCTION(BlueprintPure, Category = "Inventory System Library")
	static UInvSys_InventoryComponent* FindInventoryComponent(AActor* InActor);

	UFUNCTION(BlueprintPure, Category="Inventory System Library", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static UInvSys_InventoryControllerComponent* GetPlayerInventoryComponent(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="Inventory System Library", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static UInvSys_InventoryHUD* GetInventoryHUD(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="Inventory System Library", meta=(WorldContext="WorldContextObject", DeterminesOutputType = OutClass, UnsafeDuringActorConstruction="true"))
	static UInvSys_InventoryHUD* GetCustomInventoryHUD(const UObject* WorldContextObject, TSubclassOf<UInvSys_InventoryHUD> OutClass);

	template<class T = UInvSys_InventoryControllerComponent>
	static T* GetPlayerInventoryComponent(const UObject* WorldContextObject)
	{
		UInvSys_InventoryControllerComponent* ICC = GetPlayerInventoryComponent(WorldContextObject);
		check(ICC && ICC->IsA<T>());
		if (ICC && ICC->IsA<T>())
		{
			return (T*)ICC;
		}
		return nullptr;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category="Inventory System Library", meta=(WorldContext="WorldContextObject", DeterminesOutputType = FragmentClass, UnsafeDuringActorConstruction="true"))
	static const UInvSys_InventoryItemFragment* FindInventoryFragment(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass);
};
