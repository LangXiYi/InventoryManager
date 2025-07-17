// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InvSys_InventoryControllerComponent.generated.h"


class UInvSys_InventoryComponent;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BASEINVENTORYSYSTEM_API UInvSys_InventoryControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInvSys_InventoryControllerComponent();

public:
	/**
	 * RPC Functions
	 **/

	/*
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_EquipItemDefinition(UInvSys_InventoryComponent* InvComp,TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_EquipItemInstance(UInvSys_InventoryComponent* InvComp,UInvSys_InventoryItemInstance* InItemInstance, FGameplayTag SlotTag);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_RestoreItemInstance(UInvSys_InventoryComponent* InvComp,UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryDragItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance);
	*/

	
public:
	/** Gets the game instance this component is a part of, this will return null if not called during normal gameplay */
	template <class T>
	T* GetGameInstance() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, UGameInstance>::Value, "'T' template parameter to GetGameInstance must be derived from UGameInstance");
		AActor* Owner = GetOwner();
		return Owner ? Owner->GetGameInstance<T>() : nullptr;
	}

	template <class T>
	T* GetGameInstanceChecked() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, UGameInstance>::Value, "'T' template parameter to GetGameInstance must be derived from UGameInstance");
		AActor* Owner = GetOwner();
		check(Owner);
		T* GameInstance = Owner->GetGameInstance<T>();
		check(GameInstance);
		return GameInstance;
	}

	/** Returns true if the owner's role is ROLE_Authority */
	bool HasAuthority() const;

	/** Returns the world's timer manager */
	class FTimerManager& GetWorldTimerManager() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
