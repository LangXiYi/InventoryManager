// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

#include "TransactionCommon.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Net/UnrealNetwork.h"

UInvSys_BaseInventoryObject::UInvSys_BaseInventoryObject()
{
	
}

void UInvSys_BaseInventoryObject::InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent,
	UObject* PreEditPayLoad)
{
	if (bIsInitInventoryObject)
	{
		return;
	}
	bIsInitInventoryObject = true;
	// 复制预设数据至当前对象
	if (bIsCopyPreEditData == false)
	{
		CopyPropertyFromPreEdit(NewInventoryComponent, PreEditPayLoad);
	}
	// 仅本地控制器创建显示效果
	if (IsLocalController())
	{
		CreateDisplayWidget(NewInventoryComponent->GetPlayerController());
	}
}

void UInvSys_BaseInventoryObject::TryRefreshOccupant()
{
}

void UInvSys_BaseInventoryObject::CreateDisplayWidget(APlayerController* PC)
{
	UE_LOG(LogInventorySystem, Log, TEXT("[%s] 正在创建显示效果。"),
		PC->HasAuthority() ? TEXT("Server") : TEXT("Client"));
}

void UInvSys_BaseInventoryObject::CopyPropertyFromPreEdit(UInvSys_InventoryComponent* NewInventoryComponent,
                                                          UObject* PreEditPayLoad)
{
	check(NewInventoryComponent);
	check(PreEditPayLoad);
	
	bIsCopyPreEditData = true;
	InventoryComponent = NewInventoryComponent;

	UE_LOG(LogInventorySystem, Log, TEXT("[%s] 正在复制预设数据: From [%s] ===> To [%s]。"),
			HasAuthority() ? TEXT("Server") : TEXT("Client"),
			*PreEditPayLoad->GetName(), *this->GetName());

	COPY_INVENTORY_OBJECT_PROPERTY(UInvSys_PreEditInventoryObject, SlotName);
}

void UInvSys_BaseInventoryObject::PostRepNotifies()
{
	UObject::PostRepNotifies();
}


FName UInvSys_BaseInventoryObject::GetSlotName() const
{
	return SlotName;
}

UInvSys_InventoryComponent* UInvSys_BaseInventoryObject::GetInventoryComponent() const
{
	return InventoryComponent;
}

bool UInvSys_BaseInventoryObject::HasAuthority() const
{
	// check(InventoryComponent)
	return InventoryComponent ? InventoryComponent->HasAuthority() : false;
}

ENetMode UInvSys_BaseInventoryObject::GetNetMode() const
{
	check(InventoryComponent)
	return InventoryComponent ? InventoryComponent->GetNetMode() : NM_Standalone;
}

bool UInvSys_BaseInventoryObject::IsLocalController() const
{
	return InventoryComponent? InventoryComponent->IsLocalController() : false;
}

AActor* UInvSys_BaseInventoryObject::GetOwningActor() const
{
	if (GetInventoryComponent())
	{
		return GetInventoryComponent()->GetOwner();
	}
	return nullptr;
}

void UInvSys_BaseInventoryObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME_CONDITION(UInvSys_BaseInventoryObject, InventoryComponent, COND_None);
}
