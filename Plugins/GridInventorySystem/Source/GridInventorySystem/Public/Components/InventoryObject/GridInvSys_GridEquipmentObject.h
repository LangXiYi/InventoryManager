// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"
#include "GridInvSys_GridEquipmentObject.generated.h"

class UGridInvSys_EquipmentSlotWidget;
/**
 * ============
 * 可装备的物品
 * ============
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_GridEquipmentObject : public UInvSys_BaseEquipmentObject
{
	GENERATED_BODY()

public:
	UGridInvSys_GridEquipmentObject();
	
public:
	/**
	 * Getter Or Setter
	 **/

	virtual void CopyPropertyFromPreEdit(UObject* PreEditPayLoad) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	/** 该装备槽支持装备的类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Equipment Object")
	EGridInvSys_InventoryItemType EquipmentSupportType;
};

/**
 * Pre Edit Inventory Object
 */

UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_PreEditGridEquipmentObject : public UInvSys_PreEditEquipmentObject
{
	GENERATED_BODY()

public:
	CONSTRUCT_INVENTORY_OBJECT(UGridInvSys_GridEquipmentObject);

	/** 该装备槽支持装备的类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Type")
	EGridInvSys_InventoryItemType EquipmentSupportType;
};
