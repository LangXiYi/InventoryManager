// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryLayoutWidget.h"

#include "BaseInventorySystem.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Widgets/InvSys_InventoryItemWidget.h"
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

bool UInvSys_InventoryLayoutWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	// TODO::如何获取这个组件呢？
	UInvSys_InventoryComponent* PlayerInvComp = GetOwningPlayer()->GetComponentByClass<UInvSys_InventoryComponent>();
	
	// 对于在容器布局内放下拖拽的物品，则将该物品返回原位置
	if (InOperation->Payload && InOperation->Payload->IsA<UInvSys_InventoryItemWidget>())
	{
		UInvSys_InventoryItemWidget* ItemWidget = Cast<UInvSys_InventoryItemWidget>(InOperation->Payload);
		UInvSys_InventoryItemInstance* LOCAL_ItemInstance = ItemWidget->GetItemInstance<UInvSys_InventoryItemInstance>();
		if (LOCAL_ItemInstance)
		{
			UInvSys_InventoryComponent* From_InvComp = LOCAL_ItemInstance->GetInventoryComponent();
			if (From_InvComp && PlayerInvComp)
			{
				PlayerInvComp->Server_CancelDragItemInstance(From_InvComp);
				// 通知当前控件的库存对象，装备新物品？添加新物品？将物品放回原来位置。
				//PlayerInvComp->Server_AddItemInstance()
				PlayerInvComp->Server_RestoreItemInstance(From_InvComp, LOCAL_ItemInstance);
				return true;
			}
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
