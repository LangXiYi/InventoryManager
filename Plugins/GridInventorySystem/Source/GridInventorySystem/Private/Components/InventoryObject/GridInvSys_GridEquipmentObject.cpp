// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/GridInvSys_GridEquipmentObject.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/NamedSlot.h"
#include "Widgets/GridInvSys_EquipmentSlotWidget.h"


UGridInvSys_GridEquipmentObject::UGridInvSys_GridEquipmentObject()
{
}

void UGridInvSys_GridEquipmentObject::CopyPropertyFromPreEdit(UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(PreEditPayLoad);
	
	COPY_INVENTORY_OBJECT_PROPERTY(UGridInvSys_PreEditGridEquipmentObject, EquipmentSupportType);
}

void UGridInvSys_GridEquipmentObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
