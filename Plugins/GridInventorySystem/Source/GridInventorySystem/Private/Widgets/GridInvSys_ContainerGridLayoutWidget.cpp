// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"

#include "BaseInventorySystem.h"
#include "GridInventorySystem.h"
#include "Components/PanelWidget.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Library/InvSys_InventorySystemLibrary.h"
#include "Widgets/GridInvSys_ContainerGridItemWidget.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"

void UGridInvSys_ContainerGridLayoutWidget::RefreshWidget()
{
	Super::RefreshWidget();

	// 初始化容器布局内的所有容器
	if (ContainerGridWidgets.IsEmpty())
	{
		ContainerGridWidgets.Empty();
		GetAllContainerGridWidgets(ContainerGridWidgets);
		for (int i = 0; i < ContainerGridWidgets.Num(); ++i)
		{
			if (UInvSys_BaseInventoryObject* InvObj = GetInventoryObject())
			{
				ContainerGridWidgets[i]->SetInventoryObject(InvObj);
				ContainerGridWidgets[i]->ConstructGridItems(i);
			}
		}
	}

	if (InventoryObject)
	{
		auto ContainerFragment = InventoryObject->FindInventoryFragment<UInvSys_InventoryFragment_Container>();
		if (ContainerFragment)
		{
			RemoveAllItemInstance();
			TArray<UInvSys_InventoryItemInstance*> AllItemInstances = ContainerFragment->GetAllItemInstance();

			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
				TEXT("刷新容器布局控件 --- { 容器标签 = %s, 当前物品数量 = %d }"),
				*GetInventoryObject()->GetInventoryObjectTag().ToString(), AllItemInstances.Num())
			
			for (UInvSys_InventoryItemInstance* TempItemInstance : AllItemInstances)
			{
				if (TempItemInstance)
				{
					AddItemInstance(TempItemInstance);
				}
			}
		}
		else
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("目标库存对象未包含容器片段"))
		}
	}
}

UGridInvSys_ContainerGridWidget* UGridInvSys_ContainerGridLayoutWidget::FindContainerGrid(int32 GridID)
{
	if (ContainerGridWidgets.IsValidIndex(GridID))
	{
		return ContainerGridWidgets[GridID];
	}
	return nullptr;
}

void UGridInvSys_ContainerGridLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto WarpAddItemFunc = [this](FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message)
	{
		if (Message.InventoryObjectTag == GetSlotTag() && Message.InvComp == GetInventoryComponent())
		{
			AddItemInstance(Message.ItemInstance);
		}
	};

	auto WarpRemoveItemFunc = [this](FGameplayTag Tag, const FInvSys_InventoryItemChangedMessage& Message)
	{
		if (Message.InventoryObjectTag == GetSlotTag() && Message.InvComp == GetInventoryComponent())
		{
			RemoveItemInstance(Message.ItemInstance);
		}
	};

	auto WarpItemPositionChangedFunc = [this](FGameplayTag Tag, const FGridInvSys_ItemPositionChangeMessage& Message)
	{
		if (Message.InvComp == GetInventoryComponent())
		{
			if (Message.OldPosition.IsValid() && Message.OldPosition.EquipSlotTag == GetSlotTag())
			{
				RemoveItemInstanceFrom(Message.ItemInstance, Message.OldPosition);
			}
			if (Message.NewPosition.IsValid() && Message.NewPosition.EquipSlotTag == GetSlotTag())
			{
				AddItemInstanceTo(Message.ItemInstance, Message.NewPosition);
			}
		}
	};

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	// OnAddItemInstanceHandle =MessageSubsystem.RegisterListener<FInvSys_InventoryItemChangedMessage>(
	// 	Inventory_Message_AddItem, MoveTemp(WarpAddItemFunc));
	//
	// OnRemoveItemInstanceHandle =MessageSubsystem.RegisterListener<FInvSys_InventoryItemChangedMessage>(
	// 	Inventory_Message_RemoveItem, MoveTemp(WarpRemoveItemFunc));

	OnItemPositionChangedHandle =MessageSubsystem.RegisterListener<FGridInvSys_ItemPositionChangeMessage>(
		Inventory_Message_ItemPositionChanged, MoveTemp(WarpItemPositionChangedFunc));
}

void UGridInvSys_ContainerGridLayoutWidget::NativeDestruct()
{
	Super::NativeDestruct();

	OnAddItemInstanceHandle.Unregister();
	OnRemoveItemInstanceHandle.Unregister();
	OnItemPositionChangedHandle.Unregister();
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridLayoutWidget::FindGridItemWidgetByPos(const FGridInvSys_ItemPosition& ItemPosition) const
{
	UGridInvSys_ContainerGridItemWidget* Result = nullptr;
	if (SlotTag == ItemPosition.EquipSlotTag)
	{
		if (ContainerGridWidgets.IsValidIndex(ItemPosition.GridID))
		{
			if (UGridInvSys_ContainerGridWidget* GridWidget = ContainerGridWidgets[ItemPosition.GridID])
			{
				Result = GridWidget->GetGridItemWidget(ItemPosition.Position);
			}
		}
	}
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG && Result == nullptr, LogInventorySystem, Warning,
		TEXT("未找到指定位置的网格控件 ---> %s"), *ItemPosition.ToString())
	return Result;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridLayoutWidget::FindGridItemWidget(
	const UInvSys_InventoryItemInstance* InItemInstance) const
{
	if (InItemInstance && InItemInstance->IsA(UGridInvSys_InventoryItemInstance::StaticClass()))
	{
		UGridInvSys_InventoryItemInstance* TempItemInstance = (UGridInvSys_InventoryItemInstance*)InItemInstance;
		check(TempItemInstance);
		return FindGridItemWidgetByPos(TempItemInstance->GetItemPosition());
	}
	return nullptr;
}

void UGridInvSys_ContainerGridLayoutWidget::GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray) const
{
	//OutArray.Empty();
	UWidget* RootWidget = GetRootWidget();
	check(RootWidget);
	Private_GetAllContainerGridWidgets(OutArray, RootWidget);
}

void UGridInvSys_ContainerGridLayoutWidget::Debug_PrintContainerAllItems()
{
	auto ContainerFragment = InventoryObject->FindInventoryFragment<UInvSys_InventoryFragment_Container>();
	if (ContainerFragment)
	{
		TArray<UInvSys_InventoryItemInstance*> AllItemInstances = ContainerFragment->GetAllItemInstance();
		if (AllItemInstances.Num() > 0)
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
				TEXT("= BEG =========================================================================="))
			for (UInvSys_InventoryItemInstance* ItemInstance : AllItemInstances)
			{
				UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
					TEXT("DisplayName = %s \t Tag = %s \t Name: %s \t OuterName = %s \t"),
					*ItemInstance->GetItemDisplayName().ToString(),
					*ItemInstance->GetSlotTag().ToString(),
					*ItemInstance->GetName(),
					*ItemInstance->GetOuter()->GetName())
			}
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log,
				TEXT("= END =========================================================================="))
		}
	}
}

void UGridInvSys_ContainerGridLayoutWidget::Private_GetAllContainerGridWidgets(
	TArray<UGridInvSys_ContainerGridWidget*>& OutArray, UWidget* Parent) const
{
	if (Parent == nullptr)
	{
		return;
	}

	// 递归结束条件：Parent 不为 PanelWidget 类型。
	UPanelWidget* PanelWidget = Cast<UPanelWidget>(Parent);
	if (PanelWidget == nullptr)
	{
		return;
	}
	
	TArray<UWidget*> AllChildren = PanelWidget->GetAllChildren();
	for (UWidget* ChildWidget : AllChildren)
	{
		if (UGridInvSys_ContainerGridWidget* GridWidget = Cast<UGridInvSys_ContainerGridWidget>(ChildWidget))
		{
			OutArray.Add(GridWidget);
			continue;
		}
		// 递归查找子集中是否存在 UContainerGridWidget
		Private_GetAllContainerGridWidgets(OutArray, ChildWidget);
	}
}


void UGridInvSys_ContainerGridLayoutWidget::AddItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	check(InItemInstance)
	if (InItemInstance == nullptr)
	{
		return;
	}
	check(InItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	if (InItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	{
		UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
		AddItemInstanceTo(GridItemInstance, GridItemInstance->GetItemPosition());
	}
}

void UGridInvSys_ContainerGridLayoutWidget::AddItemInstanceTo(
	UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPosition)
{
	if (InItemInstance == nullptr)
	{
		check(false)
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品添加失败，物品实例为空"))
		return;
	}
	if (InPosition.IsValid() == false)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品添加失败，传入的位置信息不对"))
		return;
	}
	if (InItemInstance->GetInventoryComponent() != InventoryComponent)
	{
		// 添加的物品必须是在同一组件下的。
		check(false)
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品添加失败，物品实例与当前控件的库存组件不一致"))
		return;
	}
	if (SlotTag != InPosition.EquipSlotTag)
	{
		check(false)
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品添加失败，物品实例与当前控件的标签不一致"))
		return;
	}

	UGridInvSys_ContainerGridItemWidget* GridItemWidget = FindGridItemWidgetByPos(InPosition);
	check(GridItemWidget)
	if (GridItemWidget)
	{
		if (GridItemWidget->IsOccupied() == false)
		{
			GridItemWidget->AddItemInstance(InItemInstance);
		}
		// bug::重复刷新出现问题！！！
		else if(InItemInstance != GridItemWidget->GetItemInstance())
		{
			UInvSys_InventoryControllerComponent* PlayerInvComp = UInvSys_InventorySystemLibrary::GetPlayerInventoryComponent(GetWorld());
			if (PlayerInvComp)
			{
				// bug 在 1x2 大小的物品替换 两个 1x1 大小的物品时会报错
				//checkNoEntry();
				// UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Error, TEXT("位置已经被其他物品占领，物品已被丢弃至世界"))
				// PlayerInvComp->Server_DropItemInstanceToWorld(InItemInstance);
				return;
			}
		}
	}
}

void UGridInvSys_ContainerGridLayoutWidget::RemoveAllItemInstance()
{
	for (UGridInvSys_ContainerGridWidget* ContainerGridWidget : ContainerGridWidgets)
	{
		if (ContainerGridWidget)
		{
			ContainerGridWidget->RemoveAllInventoryItem();
		}
	}
}

void UGridInvSys_ContainerGridLayoutWidget::RemoveItemInstance(UInvSys_InventoryItemInstance* InItemInstance)
{
	if (InItemInstance == nullptr)
	{
		return;
	}
	if (InItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	{
		UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
		RemoveItemInstanceFrom(GridItemInstance, GridItemInstance->GetLastItemPosition());
	}
	else
	{
		// 寻找内部所有的网格，判断物品在容器内是否存在
		for (UGridInvSys_ContainerGridWidget* ContainerGridWidget : ContainerGridWidgets)
		{
			TArray<UGridInvSys_ContainerGridItemWidget*> GridItemWidgets = ContainerGridWidget->GetAllContainerGridItems();
			for (UGridInvSys_ContainerGridItemWidget* GridItemWidget : GridItemWidgets)
			{
				if (GridItemWidget && GridItemWidget->GetItemInstance() == InItemInstance)
				{
					GridItemWidget->RemoveItemInstance();
					return;
				}
			}
		}
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("移除物品失败，指定的物品实例在容器内部不存在！"))
	}
}

void UGridInvSys_ContainerGridLayoutWidget::RemoveItemInstanceFrom(UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPosition)
{
	if (InItemInstance == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品移除失败，传入的物品实例为空"))
		return;
	}
	if (InPosition.IsValid() == false)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("物品移除失败，传入的位置信息不对"))
		return;
	}

	UGridInvSys_ContainerGridItemWidget* GridItemWidget = FindGridItemWidgetByPos(InPosition);
	if (GridItemWidget && GridItemWidget->GetItemInstance() == InItemInstance)
	{
		GridItemWidget->RemoveItemInstance();
	}
}
