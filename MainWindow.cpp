#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QSplitter>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), parque(new ParqueEolico(this))
{
    setupUI();
    actualizarInterfaz();
    connect(parque, &ParqueEolico::datosActualizados, this, &MainWindow::actualizarInterfaz);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    // ===== Panel izquierdo (controles) =====
    QWidget *panelControl = new QWidget();
    QVBoxLayout *controlLayout = new QVBoxLayout(panelControl);

    // --- Grupo agregar turbina ---
    QGroupBox *grupoAgregar = new QGroupBox("Agregar nueva turbina");
    QFormLayout *formLayout = new QFormLayout(grupoAgregar);

    spinId = new QSpinBox();
    spinId->setRange(1, 9999);
    spinVelInicial = new QDoubleSpinBox();
    spinVelInicial->setRange(0.0, 30.0);
    spinVelInicial->setSingleStep(0.5);
    spinVelInicial->setValue(5.0);

    btnAgregar = new QPushButton("Agregar turbina");

    formLayout->addRow("ID:", spinId);
    formLayout->addRow("Velocidad inicial (m/s):", spinVelInicial);
    formLayout->addRow(btnAgregar);

    // --- Lista de turbinas ---
    QLabel *labelLista = new QLabel("Turbinas en el parque:");
    listaTurbinas = new QListWidget();
    listaTurbinas->setSelectionMode(QAbstractItemView::SingleSelection);

    btnEliminar = new QPushButton("Eliminar turbina seleccionada");
    btnEliminar->setEnabled(false);

    // --- Información general ---
    labelInfoGeneral = new QLabel();
    labelInfoGeneral->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    labelInfoGeneral->setAlignment(Qt::AlignTop);
    labelInfoGeneral->setWordWrap(true);

    // Armar layout del panel izquierdo
    controlLayout->addWidget(grupoAgregar);
    controlLayout->addWidget(labelLista);
    controlLayout->addWidget(listaTurbinas);
    controlLayout->addWidget(btnEliminar);
    controlLayout->addWidget(labelInfoGeneral);
    controlLayout->addStretch();

    // ===== Panel derecho (Vista 3D) =====
    m_vista3D = new Vista3D(parque, QApplication::primaryScreen());
    m_3dContainer = QWidget::createWindowContainer(m_vista3D, this);
    m_3dContainer->setMinimumSize(500, 400);

    // ===== Splitter horizontal =====
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(panelControl);
    splitter->addWidget(m_3dContainer);
    splitter->setSizes({400, 600});

    mainLayout->addWidget(splitter);

    // ===== Conexiones de señales =====
    connect(btnAgregar, &QPushButton::clicked, this, &MainWindow::agregarTurbina);
    connect(btnEliminar, &QPushButton::clicked, this, &MainWindow::eliminarTurbina);
    connect(listaTurbinas, &QListWidget::itemSelectionChanged, [this]() {
        btnEliminar->setEnabled(listaTurbinas->currentItem() != nullptr);
    });

    setWindowTitle("Gemelo Digital - Parque Eólico 3D");
    resize(1100, 700);
}

void MainWindow::agregarTurbina()
{
    int id = spinId->value();
    double vel = spinVelInicial->value();
    if (parque->buscarTurbina(id)) {
        QMessageBox::warning(this, "Error", QString("Ya existe una turbina con ID %1").arg(id));
        return;
    }
    parque->agregarTurbina(id, vel);
    spinId->setValue(spinId->value() + 1);
}

void MainWindow::eliminarTurbina()
{
    QListWidgetItem *item = listaTurbinas->currentItem();
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    parque->eliminarTurbina(id);
}

void MainWindow::actualizarInterfaz()
{
    listaTurbinas->clear();
    for (TurbinaEolica *t : parque->getTurbinas()) {
        QString texto = QString("ID %1 | Vel: %2 m/s | Pot: %3 kW | Estado: %4")
        .arg(t->getId())
            .arg(t->getVelocidadViento())
            .arg(t->calcularPotencia())
            .arg(t->getEstado());
        QListWidgetItem *item = new QListWidgetItem(texto);
        item->setData(Qt::UserRole, t->getId());
        listaTurbinas->addItem(item);
    }
    double total = parque->calcularProduccionTotal();
    QString info = QString("Cantidad de turbinas: %1\nProducción total actual: %2 kW\n\n%3")
                       .arg(parque->getCantidadTurbinas())
                       .arg(total)
                       .arg(parque->mostrarResumen());
    labelInfoGeneral->setText(info);
    btnEliminar->setEnabled(listaTurbinas->count() > 0);
}