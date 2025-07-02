// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_BaseInventoryObject.h"
#include "InvSys_CommonType.h"
#include "InvSys_BaseEquipmentObject.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_BaseEquipmentObject : public UInvSys_BaseInventoryObject
{
	GENERATED_BODY()

public:
	UInvSys_BaseEquipmentObject();

	virtual void RefreshInventoryObject(const FString& Reason = "") override;

	virtual void InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent, UObject* PreEditPayLoad) override;
	
	virtual void AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem);

	virtual void UnEquipInventoryItem();

protected:
	/**	刷新显示效果 */
	virtual void TryRefreshOccupant(const FString& Reason = "");

public:
	/**
	 * Getter Or Setter
	 **/

	virtual bool ContainsItem(FName UniqueID) override;

	FInvSys_InventoryItem GetOccupantData() const;

	bool IsEquipped() const
	{
		return bIsOccupied;
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// 装备的物品
	UPROPERTY(ReplicatedUsing=OnRep_Occupant, BlueprintReadOnly, Category = "Inventory Equipment Object")
	FInvSys_InventoryItem Occupant;
	UFUNCTION()
	virtual void OnRep_Occupant(FInvSys_InventoryItem OldOccupant);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Equipment Object")
	bool bIsOccupied = false;
};

/**
 * Pre Edit Inventory Object
 */

UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_PreEditEquipmentObject : public UInvSys_PreEditInventoryObject
{
	GENERATED_BODY()

public:
	/** 容器构建函数 */
	CONSTRUCT_INVENTORY_OBJECT(UInvSys_BaseEquipmentObject);
};