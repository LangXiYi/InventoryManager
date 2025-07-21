// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/InvSys_InventoryItemDefinition.h"

UInvSys_InventoryItemDefinition::UInvSys_InventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
{
}

const TArray<TObjectPtr<UInvSys_InventoryItemFragment>>& UInvSys_InventoryItemDefinition::GetFragments() const
{
	return Fragments;
}

const UInvSys_InventoryItemFragment* UInvSys_InventoryItemDefinition::FindFragmentByClass(
	TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UInvSys_InventoryItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}
	return nullptr;
}
