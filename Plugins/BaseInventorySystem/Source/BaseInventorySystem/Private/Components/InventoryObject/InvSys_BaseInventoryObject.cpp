// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"

#include "BaseInventorySystem.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Components/InventoryObject/Fragment/InvSys_BaseInventoryFragment.h"
#include "Data/InvSys_InventoryContentMapping.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UInvSys_BaseInventoryObject::UInvSys_BaseInventoryObject()
{
	UObject* MyOuter = GetOuter();
	if (MyOuter && IsValid(MyOuter))
	{
		if (MyOuter->IsA<UInvSys_InventoryComponent>())
		{
			InventoryComponent = Cast<UInvSys_InventoryComponent>(MyOuter);
			Owner_Private = InventoryComponent->GetOwner();
		}
	}
}

void UInvSys_BaseInventoryObject::PostRepNotifies()
{
	UObject::PostRepNotifies();
	for (int i = 0; i < InventoryObjectFragments.Num(); ++i)
	{
		if (InventoryObjectFragments[i] != nullptr)
		{
			InventoryObjectFragments[i]->InventoryObject = this;
			InventoryObjectFragments[i]->InventoryObjectTag = GetInventoryObjectTag();
		}
	}
}

void UInvSys_BaseInventoryObject::ConstructInventoryFragment(const TArray<UInvSys_BaseInventoryFragment*>& Fragments)
{
	InventoryObjectFragments.Empty();
	InventoryObjectFragments.Reserve(Fragments.Num());
	for (UInvSys_BaseInventoryFragment* Fragment : Fragments)
	{
		if (Fragment != nullptr)
		{
			// 这里不能使用 DuplicateObject，该操作会导致 ActorChanel 无法复制这些对象！！
			UInvSys_BaseInventoryFragment* TargetFragment =
				NewObject<UInvSys_BaseInventoryFragment>(GetInventoryComponent(), Fragment->GetClass());

			TargetFragment->InventoryObject = this;
			TargetFragment->InventoryObjectTag = GetInventoryObjectTag();
			InventoryObjectFragments.Emplace(TargetFragment);
		}
	}
}

void UInvSys_BaseInventoryObject::InitInventoryObject(UInvSys_InventoryObjectContent* InventoryObjectContent)
{
	check(InventoryObjectContent)
	if (InventoryObjectContent)
	{
		// 对数组进行排序，确定内部片段的执行顺序
		// InventoryObjectFragments.Sort();
		// InventoryObjectContent->Fragments.Sort();
		for (int i = 0; i < InventoryObjectContent->Fragments.Num(); ++i)
		{
			if (InventoryObjectFragments[i] != nullptr)
			{
				InventoryObjectFragments[i]->InitInventoryFragment(InventoryObjectContent->Fragments[i]);
			}
		}
	}
}

void UInvSys_BaseInventoryObject::RefreshInventoryObject()
{
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
		TEXT("[%s] 刷新库存对象及其所有片段 [%d]"),
		HasAuthority() ? TEXT("Server"):TEXT("Client"), InventoryObjectFragments.Num())
	for (UInvSys_BaseInventoryFragment* ObjectFragment : InventoryObjectFragments)
	{
		ObjectFragment->RefreshInventoryFragment();
	}
}

void UInvSys_BaseInventoryObject::RefreshInventoryFragment(TSubclassOf<UInvSys_BaseInventoryFragment> OutClass)
{
	UInvSys_BaseInventoryFragment* InventoryFragment = FindInventoryFragment(OutClass);
	if (InventoryFragment)
	{
		InventoryFragment->RefreshInventoryFragment();
	}
}

void UInvSys_BaseInventoryObject::AddInventoryFragment(UInvSys_BaseInventoryFragment* NewFragment)
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

UInvSys_BaseInventoryFragment* UInvSys_BaseInventoryObject::FindInventoryFragment(
	TSubclassOf<UInvSys_BaseInventoryFragment> OutClass)
{
	for (UInvSys_BaseInventoryFragment* Fragment : InventoryObjectFragments)
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
	for (UInvSys_BaseInventoryFragment* Fragment : InventoryObjectFragments)
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
	check(InventoryObjectTag.IsValid())
	return InventoryObjectTag;
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

	DOREPLIFETIME_CONDITION(UInvSys_BaseInventoryObject, InventoryObjectTag, COND_None);
	DOREPLIFETIME_CONDITION(UInvSys_BaseInventoryObject, InventoryObjectFragments, COND_None);
}