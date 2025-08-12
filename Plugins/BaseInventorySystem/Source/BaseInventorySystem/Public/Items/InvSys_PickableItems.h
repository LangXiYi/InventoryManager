// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
class UInvSys_InventoryItemFragment;
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

	virtual void BeginPlay() override;

	void InitItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Pickable Items")
	virtual bool PickupItem(UInvSys_InventoryComponent* InvComp);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Pickable Items")
	void InitPickableItems(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 NewStackCount);

	/** 根据 ItemDefinition 的 CDO 获取指定类型的片段信息 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UInvSys_InventoryItemFragment* FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const;

	/** 根据 ItemDefinition 的 CDO 获取指定类型的片段信息 */
	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickable Items")
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickable Items")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickable Items")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Pickable Items", meta = (ExposeOnSpawn))
	int32 ItemStackCount;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Pickable Items", meta = (ExposeOnSpawn))
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition;
};
