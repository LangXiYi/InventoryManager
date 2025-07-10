// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"

#include "BaseInventorySystem.h"
#include "Blueprint/UserWidget.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/InvSys_EquipSlotWidget.h"

UInvSys_BaseEquipmentObject::UInvSys_BaseEquipmentObject()
{
}

void UInvSys_BaseEquipmentObject::RefreshInventoryObject(const FString& Reason)
{
	Super::RefreshInventoryObject(Reason);
	TryRefreshEquipSlot();
}

void UInvSys_BaseEquipmentObject::InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent,
                                                      UObject* PreEditPayLoad)
{
	Super::InitInventoryObject(NewInventoryComponent, PreEditPayLoad);
	TryRefreshEquipSlot();
}

void UInvSys_BaseEquipmentObject::AddInventoryItemToEquipSlot_DEPRECATED(const FInvSys_InventoryItem& NewItem)
{
}

void UInvSys_BaseEquipmentObject::EquipInventoryItem(UInvSys_InventoryItemInstance* NewItemInstance)
{
	if (HasAuthority())
	{
		EquipItemInstance = NewItemInstance;
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRep_EquipItemInstance();
		}
	}
}

void UInvSys_BaseEquipmentObject::EquipInventoryItem(TSubclassOf<UInvSys_InventoryItemDefinition> NewItemDefinition)
{
	if (HasAuthority())
	{
		UInvSys_InventoryItemInstance* TempItemInstance = NewObject<UInvSys_InventoryItemInstance>(GetOwner());
		TempItemInstance->SetItemDefinition(NewItemDefinition);
		TempItemInstance->SetItemUniqueID(FGuid::NewGuid());
		if (TempItemInstance)
		{
			EquipInventoryItem(TempItemInstance);
		}
	}
}

void UInvSys_BaseEquipmentObject::UnEquipInventoryItem()
{
	if (HasAuthority())
	{
		EquipItemInstance = nullptr;
		if (GetNetMode() != NM_DedicatedServer)
		{
			OnRep_EquipItemInstance();
		}
	}
}

UInvSys_EquipSlotWidget* UInvSys_BaseEquipmentObject::CreateEquipSlotWidget(APlayerController* PC)
{
	if (PC != nullptr && PC->IsLocalController())
	{
		EquipSlotWidget = CreateWidget<UInvSys_EquipSlotWidget>(PC, EquipSlotWidgetClass);
		EquipSlotWidget->SetInventoryObject(this);
		TryRefreshEquipSlot();
	}
	return EquipSlotWidget;
}

void UInvSys_BaseEquipmentObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInvSys_BaseEquipmentObject, EquipItemInstance);
}

void UInvSys_BaseEquipmentObject::TryRefreshEquipSlot(const FString& Reason)
{
	if (EquipSlotWidget && EquipItemInstance)
	{
		EquipSlotWidget->UpdateEquipItem(EquipItemInstance);
	}
}

void UInvSys_BaseEquipmentObject::CopyPropertyFromPreEdit(UObject* PreEditPayLoad)
{
	Super::CopyPropertyFromPreEdit(PreEditPayLoad);

	COPY_INVENTORY_OBJECT_PROPERTY(UInvSys_PreEditEquipmentObject, EquipSlotWidgetClass);
}

bool UInvSys_BaseEquipmentObject::ContainsItem(FName UniqueID)
{
	return false;
	//return GetOccupantData().UniqueID == UniqueID;
}

bool UInvSys_BaseEquipmentObject::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (EquipItemInstance && IsValid(EquipItemInstance))
	{
		WroteSomething |= Channel->ReplicateSubobject(EquipItemInstance, *Bunch, *RepFlags);
	}
	return WroteSomething;
}

void UInvSys_BaseEquipmentObject::OnRep_EquipItemInstance()
{
	TryRefreshEquipSlot();
}
