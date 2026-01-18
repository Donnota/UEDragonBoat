// Fill out your copyright notice in the Description page of Project Settings。

#include "DragonBoatRaceManager.h"
#include "DragonBoatPawn.h"
#include "Datamanagement.h"
#include "Kismet/GameplayStatics.h"

ADragonBoatRaceManager::ADragonBoatRaceManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// 默认生成位置（可在编辑器中调整）
	PlayerSpawnLocation = FVector(0.0f, -500.0f, 100.0f);
	AI1SpawnLocation = FVector(0.0f, 0.0f, 100.0f);
	AI2SpawnLocation = FVector(0.0f, 500.0f, 100.0f);

	// 默认Buff数值
	SpeedUpAmount = 100.0f;
	SlowDownAmount = 100.0f;

	// 运行时状态
	PlayerBoat = nullptr;
	AI1Boat = nullptr;
	AI2Boat = nullptr;
	FinishedBoatCount = 0;
	bRaceFinished = false;
}

void ADragonBoatRaceManager::BeginPlay()
{
	Super::BeginPlay();
	
	// 自动查找Datamanagement（如果未手动指定）
	if (!DataManager)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADatamanagement::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			DataManager = Cast<ADatamanagement>(FoundActors[0]);
			UE_LOG(LogTemp, Log, TEXT("DragonBoatRaceManager: Auto-found DataManager"));
		}
	}

	// 绑定到Datamanagement的事件
	BindToDataManager();

	// 初始化比赛
	InitializeRace();
}

void ADragonBoatRaceManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADragonBoatRaceManager::InitializeRace()
{
	UE_LOG(LogTemp, Warning, TEXT("=== Initializing Dragon Boat Race ==="));

	// 检查龙舟类是否设置
	if (!DragonBoatClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DragonBoatClass is not set! Please assign it in the editor."));
		return;
	}

	// 生成3条龙舟
	PlayerBoat = SpawnBoat(PlayerSpawnLocation, EBoatType::Player, true);
	AI1Boat = SpawnBoat(AI1SpawnLocation, EBoatType::AI_1, false);
	AI2Boat = SpawnBoat(AI2SpawnLocation, EBoatType::AI_2, false);

	// 检查生成是否成功
	if (!PlayerBoat || !AI1Boat || !AI2Boat)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn all boats!"));
		return;
	}

	// 重置比赛状态
	FinishedBoatCount = 0;
	bRaceFinished = false;

	UE_LOG(LogTemp, Warning, TEXT("Race initialized successfully! 3 boats spawned."));

	// 触发比赛开始事件
	OnRaceStarted();
}

ADragonBoatPawn* ADragonBoatRaceManager::SpawnBoat(const FVector& Location, EBoatType BoatType, bool bEnableCamera)
{
	if (!GetWorld())
		return nullptr;

	// 设置生成参数
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 生成龙舟
	FRotator Rotation = FRotator::ZeroRotator;
	ADragonBoatPawn* NewBoat = GetWorld()->SpawnActor<ADragonBoatPawn>(DragonBoatClass, Location, Rotation, SpawnParams);

	if (NewBoat)
	{
		// 初始化龙舟
		NewBoat->InitializeBoat(BoatType, bEnableCamera);
		
		UE_LOG(LogTemp, Log, TEXT("Spawned DragonBoat: Type=%d, Location=%s"), 
			(int32)BoatType, *Location.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn DragonBoat at %s"), *Location.ToString());
	}

	return NewBoat;
}

void ADragonBoatRaceManager::BindToDataManager()
{
	if (!DataManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("DataManager not found! Special effects won't work."));
		return;
	}

	// TODO: 由于OnMatchesCleared是BlueprintImplementableEvent，无法直接在C++中绑定
	// 需要通过蓝图或创建一个C++可绑定的事件版本

	UE_LOG(LogTemp, Log, TEXT("DragonBoatRaceManager: DataManager connected"));
}

void ADragonBoatRaceManager::HandleMatchesCleared(const TArray<int32>& ClearedIndices, const TArray<FSpecialEffectData>& TriggeredEffects)
{
	UE_LOG(LogTemp, Log, TEXT("HandleMatchesCleared: %d effects triggered"), TriggeredEffects.Num());

	// 应用每个特殊效果
	for (const FSpecialEffectData& Effect : TriggeredEffects)
	{
		ApplySpecialEffect(Effect.EffectType);
	}
}

void ADragonBoatRaceManager::ApplySpecialEffect(ESlotEffectType EffectType)
{
	if (!PlayerBoat || !AI1Boat || !AI2Boat)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot apply effect - boats not spawned!"));
		return;
	}

	switch (EffectType)
	{
	case ESlotEffectType::SpeedUpSelf:
		// 加速玩家龙舟
		PlayerBoat->AddSpeedBuff(SpeedUpAmount);
		UE_LOG(LogTemp, Warning, TEXT("[EFFECT] Speed Up Player! (+%.1f)"), SpeedUpAmount);
		break;

	case ESlotEffectType::SlowDownEnemy:
		// 减速AI龙舟
		AI1Boat->AddSpeedBuff(-SlowDownAmount);
		AI2Boat->AddSpeedBuff(-SlowDownAmount);
		UE_LOG(LogTemp, Warning, TEXT("[EFFECT] Slow Down AI Boats! (-%.1f)"), SlowDownAmount);
		break;

	case ESlotEffectType::MoraleBoost:
		// 士气提升（已经在Datamanagement中处理）
		UE_LOG(LogTemp, Log, TEXT("[EFFECT] Morale Boost (handled by Datamanagement)"));
		break;

	default:
		break;
	}
}

void ADragonBoatRaceManager::OnBoatReachedFinish(ADragonBoatPawn* Boat)
{
	if (!Boat || Boat->bReachedFinish)
		return;

	// 增加排名计数
	FinishedBoatCount++;

	// 标记龙舟为完成
	Boat->MarkFinished(FinishedBoatCount);

	UE_LOG(LogTemp, Warning, TEXT("=== Boat Finished! Rank %d ==="), FinishedBoatCount);

	// 触发龙舟完成事件
	OnBoatFinished(Boat, FinishedBoatCount);

	// 检查是否所有龙舟都完成
	if (FinishedBoatCount >= 3)
	{
		bRaceFinished = true;
		UE_LOG(LogTemp, Warning, TEXT("=== RACE COMPLETED ==="));
		OnRaceCompleted();
	}
}

TArray<ADragonBoatPawn*> ADragonBoatRaceManager::GetAllBoats() const
{
	TArray<ADragonBoatPawn*> Boats;
	if (PlayerBoat) Boats.Add(PlayerBoat);
	if (AI1Boat) Boats.Add(AI1Boat);
	if (AI2Boat) Boats.Add(AI2Boat);
	return Boats;
}
