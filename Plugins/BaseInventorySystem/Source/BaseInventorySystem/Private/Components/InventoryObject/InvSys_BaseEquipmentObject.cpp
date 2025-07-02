// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"

#include "Net/UnrealNetwork.h"

UInvSys_BaseEquipmentObject::UInvSys_BaseEquipmentObject()
{
}

void UInvSys_BaseEquipmentObject::RefreshInventoryObject(const FString& Reason)
{
	Super::RefreshInventoryObject(Reason);
	TryRefreshOccupant();
}

void UInvSys_BaseEquipmentObject::InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent,
                                                      UObject* PreEditPayLoad)
{
	Super::InitInventoryObject(NewInventoryComponent, PreEditPayLoad);
	TryRefreshOccupant();
}

void UInvSys_BaseEquipmentObject::AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem)
{
	if (HasAuthority())
	{
		Occupant = NewItem;
		bIsOccupied = true;
		if (GetNetMode() != NM_DedicatedServer && IsLocallyControlled())
		{
			OnRep_Occupant(Occupant);
		}
	}
}

void UInvSys_BaseEquipmentObject::UnEquipInventoryItem()
{
	if (HasAuthority())
	{
		Occupant = FInvSys_InventoryItem();
		bIsOccupied = false;
		if (GetNetMode() != NM_DedicatedServer && IsLocallyControlled())
		{
			OnRep_Occupant(Occupant);
		}
	}
}

void UInvSys_BaseEquipmentObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInvSys_BaseEquipmentObject, Occupant, COND_None);
	DOREPLIFETIME_CONDITION(UInvSys_BaseEquipmentObject, bIsOccupied, COND_None);
}

void UInvSys_BaseEquipmentObject::TryRefreshOccupant(const FString& Reason)
{
	if (Reason != "") UE_LOG(LogInventorySystem, Log, TEXT("[%s]"), *Reason);
}

bool UInvSys_BaseEquipmentObject::ContainsItem(FName UniqueID)
{
	return GetOccupantData().UniqueID == UniqueID;
}

FInvSys_InventoryItem UInvSys_BaseEquipmentObject::GetOccupantData() const
{
	return Occupant;
}

void UInvSys_BaseEquipmentObject::OnRep_Occupant(FInvSys_InventoryItem OldOccupant)
{
	TryRefreshOccupant("OnRep_Occupant() ===> TryRefreshOccupant()");
}
