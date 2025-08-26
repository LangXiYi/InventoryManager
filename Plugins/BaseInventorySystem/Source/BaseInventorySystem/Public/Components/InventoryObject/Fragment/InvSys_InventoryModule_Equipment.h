// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryModule.h"
#include "InvSys_InventoryModule_Equipment.generated.h"

class UInvSys_InventoryItemDefinition;
class UInvSys_InventoryItemInstance;

USTRUCT(BlueprintType)
struct FInvSys_EquipItemInstanceMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TObjectPtr<UInvSys_InventoryComponent> InvComp = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	TObjectPtr<UInvSys_InventoryItemInstance> ItemInstance = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FGameplayTag InventoryObjectTag;
};

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryModule_Equipment : public UInvSys_InventoryModule
{
	GENERATED_BODY()

public:
	UInvSys_InventoryModule_Equipment();

	virtual void RefreshInventoryFragment() override;

	virtual void InitInventoryFragment(UObject* PreEditFragment) override;
	
	virtual UInvSys_InventoryItemInstance* EquipItemDefinition(
		TSubclassOf<UInvSys_InventoryItemInstance> ItemClass,
		TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount = 1);

	virtual UInvSys_InventoryItemInstance* EquipItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	virtual bool UnEquipItemInstance();

	UFUNCTION(BlueprintPure, Category = "Inventory Module|Equipment")
	bool HasEquipmentItems() const;

	UInvSys_InventoryItemInstance* GetEquipItemInstance() const;

protected:
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// todo:: UFUNCTION(Broadcast) BroadCast_XXX
	void BroadcastEquipItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);
	void BroadcastUnEquipItemInstance();

protected:
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Fragment")
	// FGameplayTag SupportEquipTag;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ItemInstance, Category = "Inventory Module|Equipment")
	TObjectPtr<UInvSys_InventoryItemInstance> EquipmentInstance;
	UFUNCTION()
	void OnRep_ItemInstance();
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Module|Equipment")
	TWeakObjectPtr<UInvSys_InventoryItemInstance> LastItemInstance;
};
