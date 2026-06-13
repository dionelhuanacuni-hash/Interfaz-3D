#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include "ParqueEolico.h"
#include "Vista3D.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void agregarTurbina();
    void eliminarTurbina();
    void actualizarInterfaz();

private:
    void setupUI();

    ParqueEolico *parque;

    // Widgets de la GUI
    QListWidget *listaTurbinas;
    QLabel *labelInfoGeneral;
    QPushButton *btnAgregar;
    QPushButton *btnEliminar;

    // Campos para nueva turbina
    QSpinBox *spinId;
    QDoubleSpinBox *spinVelInicial;

    // Vista 3D
    QWidget *m_3dContainer;
    Vista3D *m_vista3D;
};

#endif // MAINWINDOW_H