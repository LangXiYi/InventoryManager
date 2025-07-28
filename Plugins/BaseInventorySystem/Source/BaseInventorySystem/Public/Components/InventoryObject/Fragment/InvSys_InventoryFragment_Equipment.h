// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_BaseInventoryFragment.h"
#include "NativeGameplayTags.h"
#include "InvSys_InventoryFragment_Equipment.generated.h"

class UInvSys_InventoryItemDefinition;
class UInvSys_InventoryItemInstance;


USTRUCT(BlueprintType)
struct FInvSys_EquipItemInstanceMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TObjectPtr<UInvSys_InventoryComponent> InvComp;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	UInvSys_InventoryItemInstance* ItemInstance;
	
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FGameplayTag InventoryObjectTag;
};

// USTRUCT(BlueprintType)
// struct FInvSys_UnEquipItemInstanceMessage
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(BlueprintReadOnly, Category = "Message")
// 	UInvSys_BaseInventoryObject* InventoryObject;
//
// 	// UPROPERTY(BlueprintReadOnly, Category = "Message")
// 	// UInvSys_InventoryItemInstance* ItemInstance;
// };

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryFragment_Equipment : public UInvSys_BaseInventoryFragment
{
	GENERATED_BODY()

public:
	UInvSys_InventoryFragment_Equipment();

	virtual void RefreshInventoryFragment() override;

	virtual void InitInventoryFragment(UObject* PreEditFragment) override;
	
	virtual UInvSys_InventoryItemInstance* EquipItemDefinition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef);

	virtual void EquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);

	virtual bool UnEquipItemInstance();

	UFUNCTION(BlueprintPure, Category = "Inventory Fragment|Equipment")
	bool HasEquipmentItems() const;

	UInvSys_InventoryItemInstance* GetEquipItemInstance() const;

protected:
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void BroadcastEquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);
	void BroadcastUnEquipItemInstance();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Fragment")
	FGameplayTag SupportEquipTag;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ItemInstance, Category = "Inventory Fragment")
	TObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Fragment")
	TObjectPtr<UInvSys_InventoryItemInstance> LastItemInstance;

	UFUNCTION()
	void OnRep_ItemInstance();
};
