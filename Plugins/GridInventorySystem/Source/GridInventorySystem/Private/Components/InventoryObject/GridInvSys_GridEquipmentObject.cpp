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

UInvSys_EquipSlotWidget* UGridInvSys_GridEquipmentObject::CreateDisplayWidget(APlayerController* PC)
{
	UInvSys_EquipSlotWidget* LOCAL_EquipSlotWidget = Super::CreateDisplayWidget(PC);
	if (LOCAL_EquipSlotWidget && LOCAL_EquipSlotWidget->IsA<UGridInvSys_EquipmentSlotWidget>())
	{
		UGridInvSys_EquipmentSlotWidget* LOCAL_GridEquipSlowWidget = Cast<UGridInvSys_EquipmentSlotWidget>(LOCAL_EquipSlotWidget);
		LOCAL_GridEquipSlowWidget->SetEquipItemType(EquipmentSupportType);
	}
	return LOCAL_EquipSlotWidget;
}
