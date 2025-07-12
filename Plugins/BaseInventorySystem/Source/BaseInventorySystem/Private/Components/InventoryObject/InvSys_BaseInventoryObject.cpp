// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

#include "BaseInventorySystem.h"
#include "InvSys_InventorySystemConfig.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/InvSys_EquipSlotWidget.h"

UInvSys_BaseInventoryObject::UInvSys_BaseInventoryObject()
{
}

void UInvSys_BaseInventoryObject::OnConstructInventoryObject(UInvSys_InventoryComponent* NewInvComp,
															 UObject* PreEditPayLoad)
{
	InventoryComponent = NewInvComp;
	
	CopyPropertyFromPreEdit(PreEditPayLoad);
}

void UInvSys_BaseInventoryObject::InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent,
                                                      UObject* PreEditPayLoad)
{
	check(NewInventoryComponent);
	if (bIsInitInventoryObject)
	{
		return;
	}
	bIsInitInventoryObject = true;

}

void UInvSys_BaseInventoryObject::RefreshInventoryObject(const FString& Reason)
{
	if (Reason.IsEmpty())
	{
		return;
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("正在执行 RefreshInventoryObject() 操作。\n\tREASON: %s"), *Reason);
}

bool UInvSys_BaseInventoryObject::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	checkNoEntry()
	return false; 
}

bool UInvSys_BaseInventoryObject::RestoreItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	checkNoEntry()
	return false; 
}

UInvSys_EquipSlotWidget* UInvSys_BaseInventoryObject::CreateDisplayWidget(APlayerController* PC)
{
	return nullptr;
}

bool UInvSys_BaseInventoryObject::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                      FReplicationFlags* RepFlags)
{
	UActorChannel::SetCurrentSubObjectOwner(InventoryComponent);
	return false;
}

bool UInvSys_BaseInventoryObject::IsReadyForReplication() const
{
	if (InventoryComponent == nullptr)
	{
		return false;
	}
	check(InventoryComponent)
	return InventoryComponent->IsReadyForReplication();
}

bool UInvSys_BaseInventoryObject::IsUsingRegisteredSubObjectList()
{
	if (InventoryComponent == nullptr)
	{
		return false;
	}
	check(InventoryComponent)
	return InventoryComponent->IsUsingRegisteredSubObjectList();
}

bool UInvSys_BaseInventoryObject::ContainsItem(FGuid ItemUniqueID)
{
	return false;
}

void UInvSys_BaseInventoryObject::CopyPropertyFromPreEdit(UObject* PreEditPayLoad)
{
	check(PreEditPayLoad);

	COPY_INVENTORY_OBJECT_PROPERTY(UInvSys_PreEditInventoryObject, EquipSlotTag);

}

UInvSys_InventoryComponent* UInvSys_BaseInventoryObject::GetInventoryComponent() const
{
	return InventoryComponent;
}

bool UInvSys_BaseInventoryObject::HasAuthority() const
{
	const AActor* Actor = GetOwner();
	check(Actor);
	return Actor->HasAuthority();
}

ENetMode UInvSys_BaseInventoryObject::GetNetMode() const
{
	const AActor* Actor = GetOwner();
	check(Actor);
	return Actor->GetNetMode();
}

AActor* UInvSys_BaseInventoryObject::GetOwner() const
{
	return Cast<AActor>(GetOuter());
}

void UInvSys_BaseInventoryObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
}
