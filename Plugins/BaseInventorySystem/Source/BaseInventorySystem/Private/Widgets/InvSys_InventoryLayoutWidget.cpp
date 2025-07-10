// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryLayoutWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Widgets/Components/InvSys_TagSlot.h"


UInvSys_TagSlot* UInvSys_InventoryLayoutWidget::FindTagSlot(FGameplayTag InSlotTag)
{
	if (TagSlots.Contains(InSlotTag))
	{
		return TagSlots[InSlotTag];
	}
	return nullptr;
}

void UInvSys_InventoryLayoutWidget::CollectAllTagSlots()
{
	TagSlots.Empty();
	WidgetTree->ForEachWidget([&] (UWidget* Widget) {
		if (Widget->IsA(UInvSys_TagSlot::StaticClass()))
		{
			UInvSys_TagSlot* TagSlot = Cast<UInvSys_TagSlot>(Widget);
			check(TagSlot)
			TagSlots.Emplace(TagSlot->GetSlotTag(), TagSlot);
		}
	});
}
