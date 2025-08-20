// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

#include "BaseInventorySystem.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryModule.h"
#include "Data/InvSys_InventoryContentMapping.h"
#include "Engine/ActorChannel.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

UInvSys_BaseInventoryObject::UInvSys_BaseInventoryObject()
{
	InventoryComponent = GetTypedOuter<UInvSys_InventoryComponent>();
	if (InventoryComponent)
	{
		Owner_Private = InventoryComponent->GetOwner();
	}
}

void UInvSys_BaseInventoryObject::PostRepNotifies()
{
	UObject::PostRepNotifies();
	for (int i = 0; i < InventoryModules.Num(); ++i)
	{
		if (InventoryModules[i] != nullptr)
		{
			InventoryModules[i]->InventoryObject = this;
			InventoryModules[i]->InventoryTag = GetInventoryObjectTag();
		}
	}
}

void UInvSys_BaseInventoryObject::ConstructInventoryFragment(const TArray<UInvSys_InventoryModule*>& Fragments)
{
	InventoryModules.Empty();
	InventoryModules.Reserve(Fragments.Num());
	for (UInvSys_InventoryModule* Fragment : Fragments)
	{
		if (Fragment != nullptr)
		{
			// 这里不能使用 DuplicateObject，该操作会导致 ActorChanel 无法复制这些对象！！
			UInvSys_InventoryModule* TargetFragment =
				NewObject<UInvSys_InventoryModule>(GetInventoryComponent(), Fragment->GetClass());

			TargetFragment->InventoryObject = this;
			TargetFragment->InventoryTag = GetInventoryObjectTag();
			InventoryModules.Emplace(TargetFragment);
		}
	}
}

void UInvSys_BaseInventoryObject::InitInventoryObject(UInvSys_InventoryComponent* InvComp, int32 InventoryObjectID)
{
	UInvSys_InventoryObjectContent* InventoryObjectContent = InvComp->GetInventoryObjectContent(InventoryObjectID);
	if (InventoryObjectContent == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, InventoryObjectContent is nullptr, Index = %d"), __FUNCTION__, InventoryObjectID);
		return;
	}
	for (int i = 0; i < InventoryObjectContent->Fragments.Num(); ++i)
	{
		if (InventoryModules[i] != nullptr)
		{
			/**
			 * 生成库存模块ID
			 * WARNING: 同一个 ActorChannel 内不得出现重复的 ID
			 */
			InventoryModules[i]->InventoryModuleID = InventoryObjectID * MAX_INVENTORY_MODULE + i;
			InventoryModules[i]->InitInventoryFragment(InventoryObjectContent->Fragments[i]);
		}
	}
}

void UInvSys_BaseInventoryObject::RefreshInventoryObject()
{
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
		TEXT("[%s] 刷新库存对象及其所有片段 [%d]"),
		HasAuthority() ? TEXT("Server"):TEXT("Client"), InventoryModules.Num())
	for (UInvSys_InventoryModule* ObjectFragment : InventoryModules)
	{
		ObjectFragment->RefreshInventoryFragment();
	}
}

void UInvSys_BaseInventoryObject::RefreshInventoryFragment(TSubclassOf<UInvSys_InventoryModule> OutClass)
{
	UInvSys_InventoryModule* InventoryFragment = FindInventoryModule(OutClass);
	if (InventoryFragment)
	{
		InventoryFragment->RefreshInventoryFragment();
	}
}

void UInvSys_BaseInventoryObject::AddInventoryFragment(UInvSys_InventoryModule* NewFragment)
{
	/*if (HasAuthority())
	{
		if (NewFragment != nullptr)
		{
			UInvSys_BaseInventoryFragment* TargetFragment =
				NewObject<UInvSys_BaseInventoryFragment>(this, NewFragment->GetClass());
			InventoryObjectFragments.Add(TargetFragment);
		}
	}*/
}

UInvSys_InventoryModule* UInvSys_BaseInventoryObject::FindInventoryModule(
	TSubclassOf<UInvSys_InventoryModule> OutClass)
{
	for (UInvSys_InventoryModule* Fragment : InventoryModules)
	{
		check(Fragment)
		if (Fragment && Fragment->IsA(OutClass))
		{
			return Fragment;
		}
	}
	return nullptr;
}

bool UInvSys_BaseInventoryObject::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                      FReplicationFlags* RepFlags)
{
	bool bWroteSomething = false;
	UActorChannel::SetCurrentSubObjectOwner(GetInventoryComponent());
	for (UInvSys_InventoryModule* Fragment : InventoryModules)
	{
		if (Fragment == nullptr || IsValid(Fragment) == false)
		{
			continue;
		}
		bWroteSomething |= Fragment->ReplicateSubobjects(Channel, Bunch, RepFlags);
		bWroteSomething |= Channel->ReplicateSubobject(Fragment, *Bunch, *RepFlags);
	}
	return bWroteSomething;
}

bool UInvSys_BaseInventoryObject::IsReadyForReplication() const
{
	check(InventoryComponent)
	return InventoryComponent->IsReadyForReplication();
}

bool UInvSys_BaseInventoryObject::IsUsingRegisteredSubObjectList() const
{
	check(InventoryComponent)
	return InventoryComponent->IsUsingRegisteredSubObjectList();
}

FGameplayTag UInvSys_BaseInventoryObject::GetInventoryObjectTag() const
{
	check(InventoryTag.IsValid())
	return InventoryTag;
}

UInvSys_InventoryComponent* UInvSys_BaseInventoryObject::GetInventoryComponent() const
{
	check(InventoryComponent);
	return InventoryComponent;
}

bool UInvSys_BaseInventoryObject::HasAuthority() const
{
	check(Owner_Private);
	return Owner_Private->HasAuthority();
}

ENetMode UInvSys_BaseInventoryObject::GetNetMode() const
{
	check(Owner_Private);
	return Owner_Private->GetNetMode();
}

AActor* UInvSys_BaseInventoryObject::GetOwner() const
{
	return Owner_Private;
}

void UInvSys_BaseInventoryObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInvSys_BaseInventoryObject, InventoryTag, COND_None);
	DOREPLIFETIME_CONDITION(UInvSys_BaseInventoryObject, InventoryModules, COND_None);
}