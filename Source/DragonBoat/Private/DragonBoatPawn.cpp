// Fill out your copyright notice in the Description page of Project Settings.

#include "DragonBoatPawn.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ADragonBoatPawn::ADragonBoatPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件（网格体）
	BoatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoatMesh"));
	RootComponent = BoatMesh;

	// 创建移动组件
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;

	// 创建弹簧臂（摄像机支架）
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->SetRelativeRotation(FRotator(-30.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;

	// 创建摄像机
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->SetActive(false); // 默认禁用，只有玩家龙舟才启用

	// 默认值
	BaseSpeed = 300.0f;
	CurrentSpeed = BaseSpeed;
	SpeedBuff = 0.0f;
	bReachedFinish = false;
	FinishRank = 0;
	BoatType = EBoatType::AI_1;
}

void ADragonBoatPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化速度
	UpdateSpeed();
}

void ADragonBoatPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 如果未到达终点，自动向前移动
	if (!bReachedFinish && MovementComponent)
	{
		// 直接设置移动速度（不依赖PlayerController）
		FVector ForwardVector = GetActorForwardVector();
		FVector Velocity = ForwardVector * CurrentSpeed;
		MovementComponent->Velocity = Velocity;

		// 调试日志（每秒输出一次）
		static float DebugTimer = 0.0f;
		DebugTimer += DeltaTime;
		if (DebugTimer >= 1.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("Boat [%d] Tick: Velocity=%s, Speed=%.1f"), 
				(int32)BoatType, *Velocity.ToString(), CurrentSpeed);
			DebugTimer = 0.0f;
		}
	}
}

void ADragonBoatPawn::InitializeBoat(EBoatType InBoatType, bool bEnableCamera)
{
	BoatType = InBoatType;

	// 启用或禁用摄像机
	if (Camera)
	{
		Camera->SetActive(bEnableCamera);
	}

	// 如果是玩家龙舟，设置为被控制
	if (bEnableCamera)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			PC->Possess(this);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DragonBoat initialized: Type=%d, Camera=%s"), 
		(int32)BoatType, bEnableCamera ? TEXT("Enabled") : TEXT("Disabled"));
}

void ADragonBoatPawn::AddSpeedBuff(float BuffAmount)
{
	if (bReachedFinish)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add speed buff - boat has finished!"));
		return;
	}

	SpeedBuff += BuffAmount;
	UpdateSpeed();

	UE_LOG(LogTemp, Log, TEXT("DragonBoat [%d] Speed Buff: %+.1f (Total Buff: %+.1f, Current Speed: %.1f)"), 
		(int32)BoatType, BuffAmount, SpeedBuff, CurrentSpeed);
}

void ADragonBoatPawn::ClearSpeedBuff()
{
	SpeedBuff = 0.0f;
	UpdateSpeed();

	UE_LOG(LogTemp, Log, TEXT("DragonBoat [%d] Speed Buff Cleared (Speed: %.1f)"), 
		(int32)BoatType, CurrentSpeed);
}

void ADragonBoatPawn::MarkFinished(int32 Rank)
{
	if (bReachedFinish)
		return;

	bReachedFinish = true;
	FinishRank = Rank;

	// 停止移动
	CurrentSpeed = 0.0f;
	if (MovementComponent)
	{
		MovementComponent->MaxSpeed = 0.0f;
	}

	UE_LOG(LogTemp, Warning, TEXT("DragonBoat [%d] FINISHED! Rank: %d"), 
		(int32)BoatType, FinishRank);
}

void ADragonBoatPawn::UpdateSpeed()
{
	// 计算当前速度 = 基础速度 + Buff
	CurrentSpeed = BaseSpeed + SpeedBuff;

	// 确保速度不为负数
	CurrentSpeed = FMath::Max(CurrentSpeed, 0.0f);

	// 更新移动组件的最大速度
	if (MovementComponent)
	{
		MovementComponent->MaxSpeed = CurrentSpeed;
	}
}
