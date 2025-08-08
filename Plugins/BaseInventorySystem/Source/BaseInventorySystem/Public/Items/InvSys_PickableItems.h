// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InvSys_PickableItems.generated.h"

class UInvSys_InventoryItemInstance;
class UInvSys_InventoryItemDefinition;
class USphereComponent;

USTRUCT(BlueprintType)
struct FDropItemMessage
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	class UInvSys_InventoryItemInstance* ItemInstance = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	class AInvSys_PickableItems* DropItem = nullptr;

	UPROPERTY(BlueprintReadOnly)
	class AActor* FromActor = nullptr;
};

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API AInvSys_PickableItems : public AActor
{
	GENERATED_BODY()

	friend class UInvSys_InventoryComponent;

public:
	AInvSys_PickableItems();

	void InitItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickable Items")
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickable Items")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickable Items")
	TObjectPtr<USphereComponent> SphereCollision;

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepItemInstance, Category = "Pickable Items", meta = (ExposeOnSpawn, AllowPrivateAccess))
	TObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;

	UFUNCTION()
	void OnRepItemInstance();
};
