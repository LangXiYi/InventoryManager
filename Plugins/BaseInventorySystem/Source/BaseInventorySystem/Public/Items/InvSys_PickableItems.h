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

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Pickable Items")
	virtual bool PickupItem(UInvSys_InventoryComponent* InvComp);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Pickable Items")
	void InitPickableItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	// 每次修改对象属性后都需要调用该函数
	void MarkItemDirty();

public:
	int32 GetItemStackCount() const;

	void SetItemStackCount(int32 NewStackCount);
	
	/** 根据 ItemDefinition 的 CDO 获取指定类型的片段信息 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UInvSys_InventoryItemFragment* FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const;

	/** 根据 ItemDefinition 的 CDO 获取指定类型的片段信息 */
	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickable Items")
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickable Items")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pickable Items")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PickableItemInstance, Category = "Pickable Items", meta = (ExposeOnSpawn))
	TObjectPtr<UInvSys_InventoryItemInstance> PickableItemInstance;
	UFUNCTION()
	void OnRep_PickableItemInstance();

private:
	bool bIsDirty = false;
};
