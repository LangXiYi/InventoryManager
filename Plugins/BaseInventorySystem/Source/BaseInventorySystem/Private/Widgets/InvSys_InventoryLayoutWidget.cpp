// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryLayoutWidget.h"

#include "BaseInventorySystem.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Library/InvSys_InventorySystemLibrary.h"
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

void UInvSys_InventoryLayoutWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([&] (UWidget* Widget) {
			if (Widget->IsA(UInvSys_TagSlot::StaticClass()))
			{
				UInvSys_TagSlot* TagSlot = Cast<UInvSys_TagSlot>(Widget);
				check(TagSlot)
				TagSlots.Emplace(TagSlot->GetSlotTag(), TagSlot);
			}
		});
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error, TEXT("重构控件时，同步收集所有命名槽！收集数量 = %d"), TagSlots.Num())
	}
}

void UInvSys_InventoryLayoutWidget::AddWidget(UUserWidget* Widget, const FGameplayTag& Tag)
{
	// 根据库存对象的标签查询对应的槽位，然后将需要显示的控件添加到目标槽位下。
	if (UInvSys_TagSlot* TagSlot = FindTagSlot(Tag))
	{
		TagSlot->AddChild(Widget);
	}
	else
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("标签为 %s 的控件在布局中未找到"), *Tag.ToString());
	}
}

bool UInvSys_InventoryLayoutWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                                 UDragDropOperation* InOperation)
{
	UInvSys_InventoryControllerComponent* ICC = UInvSys_InventorySystemLibrary::GetPlayerInventoryComponent(GetWorld());
	check(ICC)
	if (ICC == nullptr)
	{
		return false;
	}
	UInvSys_InventoryItemInstance* LOCAL_ItemInstance = Cast<UInvSys_InventoryItemInstance>(InOperation->Payload);
	check(LOCAL_ItemInstance)
	if (LOCAL_ItemInstance)
	{
		UInvSys_InventoryComponent* From_InvComp = LOCAL_ItemInstance->GetInventoryComponent();
		if (From_InvComp && ICC)
		{
			// 对于在容器布局内放下拖拽的物品，则将该物品返回原位置
			ICC->Server_RestoreItemInstance(From_InvComp, LOCAL_ItemInstance);
			return true;
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
